#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/eventfd.h>

#include "utils/logger.h"
#include "utils/queue.h"

#include "app/app_rs485.h"

#define DEVICE_PATH "/dev/ttySTM1" // 设备路径

#define EPOLL_SIZE 2 // 监听事件数量(串口 fd 和 stop_fd)

#define BUF_LEN 1024 // 接收buffer
uint8_t rx_buf[BUF_LEN];

struct rs485_dev {
	int fd;					  // 串口文件描述符
	int epoll_fd;			  // epoll监听fd
	int stop_fd;			  // 接收关闭信号
	struct queue_info rx_q;	  // 接收队列
	pthread_t thread;		  // 接收线程
	pthread_rwlock_t rw_lock; // 读写锁
	bool running;			  // 运行标志
};

struct rs485_dev *g_485 = NULL;

/**
 * @brief 初始化串口
 * 
 * @param fd 文件描述符
 * @return bool 
 */
bool serial_init(int fd)
{
	struct termios options;

	// 获取当前串口配置
	if (tcgetattr(fd, &options) != 0) {
		LOG_E("tcgetattr failed: %s", strerror(errno));
		return false;
	}

	// 设置串口基本配置
	options.c_cflag |= (CLOCAL | CREAD); // 启用串口接收和本地模式
	options.c_cflag &= ~CSIZE;			 // 清除数据位设置
	options.c_cflag &= ~CRTSCTS;		 // 禁用硬件流控制
	options.c_cflag |= CS8;				 // 设置数据位为8位
	options.c_cflag &= ~CSTOPB;			 // 设置停止位为1位
	options.c_iflag |= IGNPAR;			 // 忽略奇偶校验错误
	options.c_oflag = 0;				 // 原始输出模式
	options.c_lflag = 0;				 // 禁用终端处理标志(如回显)

	// 波特率115200
	if (cfsetispeed(&options, B115200) != 0 || cfsetospeed(&options, B115200) != 0) {
		LOG_E("Setting baud rate failed: %s", strerror(errno));
		return false;
	}

	// 应用设置
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		LOG_E("tcsetattr failed: %s", strerror(errno));
		return false;
	}

	return true;
}

// 不断读取的线程
void *read_thread(void *arg)
{
	struct rs485_dev *app_485 = arg;

	struct epoll_event events[EPOLL_SIZE];

	while (app_485->running) {
		int nfds = epoll_wait(app_485->epoll_fd, events, EPOLL_SIZE, -1);
		if (nfds < 0) {
			if (errno == EINTR)
				continue;
			LOG_E("epoll_wait failed: %s", strerror(errno));
			break;
		}

		for (int i = 0; i < nfds; i++) {
			if (events[i].data.fd == app_485->stop_fd) {
				// 收到关闭信号,退出线程
				uint64_t val;
				ssize_t s = read(app_485->stop_fd, &val, sizeof(val));
				if (s != sizeof(val))
					LOG_E("Failed to read from eventfd: %s", strerror(errno));
				goto exit_thread;
			}

			if (events[i].events & EPOLLIN) {
				// 可读事件
				uint8_t temp_buf[1024] = { 0 };

				pthread_rwlock_rdlock(&app_485->rw_lock);
				ssize_t read_len = read(app_485->fd, temp_buf, sizeof(temp_buf));
				pthread_rwlock_unlock(&app_485->rw_lock);

				if (read_len < 0) {
					LOG_E("Read failed: %s", strerror(errno));
					continue;
				}
				queue_add(&app_485->rx_q, temp_buf, read_len); // 读取的数据加入队列
			}
		}
	}

exit_thread:
	return NULL;
}

/**
 * @brief RS485初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_rs485_init(void **p_priv)
{
	bool ret;

	struct rs485_dev *app_485 = malloc(sizeof(struct rs485_dev));
	if (!app_485) {
		LOG_E("Malloc 485 failed");
		return false;
	}
	app_485->running = true;

	int res = pthread_rwlock_init(&app_485->rw_lock, NULL);
	if (res != 0) {
		LOG_E("Init rwlock failed");
		goto err_free_485;
	}

	// 打开串口
	app_485->fd = open(DEVICE_PATH, O_RDWR);
	if (app_485->fd < 0) {
		LOG_E("Open device:%s failed\n", DEVICE_PATH);
		goto err_free_rwlock;
	}

	// 配置串口
	ret = serial_init(app_485->fd);
	if (!ret)
		goto err_close_fd;

	// 创建 epoll 实例
	app_485->epoll_fd = epoll_create1(0);
	if (app_485->epoll_fd < 0) {
		LOG_E("epoll_create1 failed: %s", strerror(errno));
		goto err_close_fd;
	}

	// 添加串口 fd 到 epoll
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = app_485->fd;
	if (epoll_ctl(app_485->epoll_fd, EPOLL_CTL_ADD, app_485->fd, &ev) < 0) {
		LOG_E("Failed to add serial fd to epoll: %s", strerror(errno));
		goto err_close_epoll;
	}

	// 创建 stop_fd
	app_485->stop_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (app_485->stop_fd < 0) {
		LOG_E("Failed to create eventfd: %s", strerror(errno));
		goto err_close_epoll;
	}

	// 添加 stop_fd 到 epoll
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = app_485->stop_fd;
	if (epoll_ctl(app_485->epoll_fd, EPOLL_CTL_ADD, app_485->stop_fd, &ev) < 0) {
		LOG_E("Failed to add eventfd to epoll: %s", strerror(errno));
		goto err_close_stop_fd;
	}

	// 初始化队列
	ret = queue_init(&app_485->rx_q, 1, rx_buf, BUF_LEN);
	if (!ret) {
		LOG_E("Init queue failed");
		goto err_close_stop_fd;
	}

	// 创建接收线程
	int thread_ret = pthread_create(&app_485->thread, NULL, read_thread, app_485);
	if (thread_ret != 0) {
		LOG_E("Create read thread failed: %s", strerror(thread_ret));
		goto err_destroy_queue;
	}

	*p_priv = app_485;
	g_485 = app_485;

	return true;

err_destroy_queue:
	queue_destroy(&app_485->rx_q);

err_close_stop_fd:
	close(app_485->stop_fd);

err_close_epoll:
	close(app_485->epoll_fd);

err_close_fd:
	close(app_485->fd);

err_free_rwlock:
	pthread_rwlock_destroy(&app_485->rw_lock);

err_free_485:
	free(app_485);

	return false;
}

/**
 * @brief 去初始化rs485
 * 
 * @param priv 私有数据指针
 */
void app_rs485_deinit(void *priv)
{
	if (!priv)
		return;

	struct rs485_dev *app_485 = priv;

	app_485->running = false;

	// 通知 read_thread 退出
	uint64_t val = 1;
	ssize_t s = write(app_485->stop_fd, &val, sizeof(val));
	if (s != sizeof(val)) {
		LOG_E("Failed to write to eventfd: %s", strerror(errno));
	}

	pthread_join(app_485->thread, NULL);

	queue_destroy(&app_485->rx_q);
	close(app_485->stop_fd);
	close(app_485->epoll_fd);
	close(app_485->fd);
	pthread_rwlock_destroy(&app_485->rw_lock);
	free(app_485);
}

/**
 * @brief rs485任务处理
 * 
 * @param priv 私有数据指针
 */
void app_rs485_task(void *priv)
{
	if (!priv)
		return;

	struct rs485_dev *app_485 = priv;

#define TEMP_READ_SIZE (256U)
	uint8_t read_buf[TEMP_READ_SIZE] = { 0 };

	size_t ret = queue_get(&app_485->rx_q, read_buf, TEMP_READ_SIZE);
	if (ret <= 0)
		return;

	for (size_t i = 0; i < ret; i++)
		printf("0x%02x ", read_buf[i]);
	printf("\n");
}

/**
 * @brief 发送数据
 * 
 * @param buf 数据缓冲
 * @param len 数据长度
 */
void app_rs485_write(uint8_t *buf, size_t len)
{
	if (!buf || !len || !g_485 || g_485->fd < 0)
		return;

	pthread_rwlock_wrlock(&g_485->rw_lock);
	if (write(g_485->fd, buf, len) < 0)
		LOG_E("Write failed: %s", strerror(errno));

	pthread_rwlock_unlock(&g_485->rw_lock);
}
