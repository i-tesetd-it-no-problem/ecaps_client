/**
 * @file app_rs485.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief RS485串口任务
 * @version 1.0
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

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
	int fd;						 // 串口文件描述符
	int epoll_fd;				 // epoll监听fd
	int stop_fd;				 // 接收关闭信号
	struct queue_info rx_q;		 // 接收队列
	pthread_t thread;			 // 接收线程
	pthread_rwlock_t rw_lock;	 // 读写锁
	bool running;				 // 运行标志
	struct termios original_tio; // 原始termios设置
};

static struct rs485_dev *g_485 = NULL;

/**
 * @brief 初始化串口
 * 
 * @param dev_rs485 rs485设备结构体指针
 * @return bool 
 */
static bool serial_init(struct rs485_dev *dev_rs485)
{
	struct termios options;

	// 获取当前串口配置并保存
	if (tcgetattr(dev_rs485->fd, &dev_rs485->original_tio) != 0) {
		LOG_E("tcgetattr failed: %s", strerror(errno));
		return false;
	}

	// 设置串口基本配置
	options = dev_rs485->original_tio;

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
	if (tcsetattr(dev_rs485->fd, TCSANOW, &options) != 0) {
		LOG_E("tcsetattr failed: %s", strerror(errno));
		return false;
	}

	return true;
}

/**
 * @brief 关闭串口并清理串口配置
 * 
 * @param dev_rs485 rs485设备结构体指针
 */
static void serial_deinit(struct rs485_dev *dev_rs485)
{
	if (dev_rs485->fd < 0)
		return;

	// 确保所有数据已传输
	if (tcdrain(dev_rs485->fd) != 0) {
		LOG_E("tcdrain failed: %s", strerror(errno));
	}

	// 清空输入和输出缓冲区
	if (tcflush(dev_rs485->fd, TCIOFLUSH) != 0) {
		LOG_E("tcflush failed: %s", strerror(errno));
	}

	// 恢复原始termios设置
	if (tcsetattr(dev_rs485->fd, TCSANOW, &dev_rs485->original_tio) != 0) {
		LOG_E("tcsetattr restore failed: %s", strerror(errno));
	}

	// 关闭串口
	close(dev_rs485->fd);
}

/**
 * @brief 接收数据线程
 * 
 * @param arg 
 * @return void* 
 */
static void *read_thread(void *arg)
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
	app_485->fd = -1;
	app_485->epoll_fd = -1;
	app_485->stop_fd = -1;

	int res = pthread_rwlock_init(&app_485->rw_lock, NULL);
	if (res != 0) {
		LOG_E("Init rwlock failed");
		goto err_free_485;
	}

	// 打开串口
	app_485->fd = open(DEVICE_PATH, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (app_485->fd < 0) {
		LOG_E("Open device:%s failed: %s", DEVICE_PATH, strerror(errno));
		goto err_free_rwlock;
	}

	// 配置串口
	ret = serial_init(app_485);
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
	serial_deinit(app_485);

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
	serial_deinit(app_485);
	pthread_rwlock_destroy(&app_485->rw_lock);
	free(app_485);
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

/**
 * @brief 测试收发是否正常
 * 
 */
static void app_rs485_test(void)
{
	// 1s发一次
	static size_t counter = 0;
	counter += APP_RS485_TASK_PERIOD;
	if (counter < 1000)
		return;

	counter = 0;
	uint8_t test_data[] = {
		0x06,
		0x03,
		0xE1,
		0xC8,
		0x00,
		0x1E,
		0x73,
		0xB7,
	};
	app_rs485_write(test_data, sizeof(test_data));
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

	app_rs485_test();

#define TEMP_READ_SIZE (256U)
	uint8_t read_buf[TEMP_READ_SIZE] = { 0 };

	size_t ret = queue_get(&app_485->rx_q, read_buf, TEMP_READ_SIZE);
	if (ret <= 0)
		return;

	for (size_t i = 0; i < ret; i++)
		printf("0x%02x ", read_buf[i]);
	printf("\n");
}
