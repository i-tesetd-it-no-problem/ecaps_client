/**
 * @file can_device.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief CAN 配置组件
 * @version 1.0
 * @date 2024-12-12
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/eventfd.h>
#include <fcntl.h>

#include "utils/logger.h"

#include "app/can_device.h"

struct can_device {
	const struct can_config *config;
	int socket_fd;	  // CAN套接字文件描述符
	int stop_fd;	  // 停止信号文件描述符
	int epfd;		  // epoll文件描述符
	pthread_t thread; // 读取线程
};

/**
 * @brief 统一清理函数
 * 
 * @param dev CAN设备结构体指针
 * @param socket_fd CAN套接字文件描述符
 */
static void cleanup(can_handle handle, int socket_fd)
{
	if (handle) {
		if (handle->epfd > 0)
			close(handle->epfd);
		if (handle->stop_fd > 0)
			close(handle->stop_fd);
		free(handle);
	}
	if (socket_fd > 0)
		close(socket_fd);
}

/**
 * @brief 构造 CAN 帧
 *
 * @param frame 指向待填充的 `struct can_frame` 结构体的指针
 * @param data 有效载荷数据
 * @param len 有效载荷长度 范围 0 到 CAN_MAX_DLEN (8)
 * @return true 构造成功
 * @return false 构造失败
 */
static bool make_one_can_frame(
	can_handle handle, struct can_frame *frame, const uint8_t *data, uint8_t len)
{
	if (frame == NULL || handle == NULL) {
		LOG_E("Invalid frame pointer");
		return false;
	}

	if (len > CAN_MAX_DLEN) {
		LOG_E("Data length %d exceeds maximum %d", len, CAN_MAX_DLEN);
		return false;
	}

	if (len > 0 && data == NULL) {
		LOG_E("Data pointer is NULL but data length is %d", len);
		return false;
	}

	frame->can_id = handle->config->can_id;
	frame->can_dlc = len;

	// 填充有效载荷数据
	if (len > 0 && data != NULL) {
		memcpy(frame->data, data, len);
		if (len < CAN_MAX_DLEN)
			memset(frame->data + len, 0, CAN_MAX_DLEN - len); // 补零
	} else {
		memset(frame->data, 0, CAN_MAX_DLEN);
	}

	return true;
}

/**
 * @brief 配置CAN过滤器
 *
 * @param socket_fd CAN套接字
 * @param filters 过滤器数组,包含多个CAN ID和掩码
 * @param filter_count 过滤器的数量
 * @return true 成功配置过滤器
 * @return false 配置过滤器失败
 */
static bool configure_can_filters(int socket_fd, struct can_filter *filters, size_t filter_count)
{
	if (!filters || filter_count == 0) {
		LOG_E("Invalid filters or filter count");
		return false;
	}

	if (setsockopt(socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, filters,
			filter_count * sizeof(struct can_filter)) < 0) {
		LOG_E("setsockopt CAN_RAW_FILTER failed: %s", strerror(errno));
		return false;
	}

	return true;
}

/**
 * @brief 读取线程函数,监听CAN套接字和停止通知
 *
 * @param arg 指向can_device结构体的指针
 * @return void* 返回NULL
 */
static void *can_read_thread_func(void *arg)
{
	if (!arg)
		return NULL;

	can_handle handle = arg;	  // can句柄
	struct epoll_event events[2]; // epoll事件数组
	struct can_frame frame;		  // 读取的CAN帧

	while (1) {
		int n = epoll_wait(handle->epfd, events, 2, -1);
		if (n < 0) {
			if (errno == EINTR)
				continue; // 被信号中断,继续等待
			LOG_E("epoll_wait failed: %s", strerror(errno));
			break;
		}

		for (int i = 0; i < n; i++) {
			if (events[i].data.fd == handle->stop_fd) {
				uint64_t u;
				if (read(handle->stop_fd, &u, sizeof(uint64_t)) < 0)
					LOG_E("Failed to read from stop_fd: %s", strerror(errno));
				return NULL; // 退出线程
			} else if (events[i].data.fd == handle->socket_fd) {
				int nbytes = read(handle->socket_fd, &frame, sizeof(struct can_frame));
				if (nbytes < 0)
					handle->config->handle(NULL);
				else if (nbytes == sizeof(struct can_frame) && handle->config &&
					handle->config->handle)
					handle->config->handle(&frame); // 接收到正常数据
				else
					handle->config->handle(NULL);
			}
		}
	}
	return NULL;
}

/**
 * @brief 配置并启用 CAN 接口
 *
 * @return true 配置成功
 * @return false 配置失败
 */
static bool setup_can_interface(const struct can_config *config)
{
	if (!config) {
		LOG_E("Invalid args");
		return false;
	}

	char cmd[256] = { 0 };
	int ret;

	// 设置 CAN 接口的比特率并启用接口
	// 格式: ip link set <CAN_DEV_NAME> up type can bitrate <CAN_BITRATE>
	snprintf(cmd, sizeof(cmd), "ip link set %s up type can bitrate %d", config->can_dev_name,
		config->can_bitrate);
	ret = system(cmd);
	if (ret != 0) {
		LOG_E("Failed to bring up CAN interface using command: %s", cmd);
		return false;
	}

	// 检查接口是否已启用
	char operstate_path[256];
	snprintf(operstate_path, sizeof(operstate_path), "/sys/class/net/%s/operstate",
		config->can_dev_name);
	int fd = open(operstate_path, O_RDONLY);
	if (fd < 0) {
		LOG_E("Failed to open operstate: %s", strerror(errno));
		return false;
	}

	char state[16] = { 0 };
	ssize_t n = read(fd, state, sizeof(state) - 1);
	close(fd);
	if (n <= 0) {
		LOG_E("Failed to read CAN interface state");
		return false;
	}
	state[strcspn(state, "\n")] = '\0';
	if (strcmp(state, "up") != 0) {
		LOG_E("CAN interface %s is not up, current state: %s", config->can_dev_name, state);
		return false;
	}

	return true;
}

/**
 * @brief 关闭 CAN 接口
 */
static void shutdown_can_interface(const struct can_config *config)
{
	if (!config) {
		LOG_E("Invalid args");
		return;
	}

	char cmd[256] = { 0 };
	snprintf(cmd, sizeof(cmd), "ip link set %s down", config->can_dev_name);
	if (system(cmd) != 0) {
		LOG_E("Failed to bring down CAN interface using command: %s", cmd);
	}
}

/**
 * @brief CAN设备初始化
 *
 * 初始化CAN套接字,设置接口,绑定套接字到CAN设备,创建epoll实例,
 * 添加CAN套接字和eventfd到epoll,并启动读取线程.
 *
 * @param config 用户配置信息
 * @return true 初始化成功
 * @return false 初始化失败
 */
can_handle can_device_init(const struct can_config *config)
{
	if (!config) {
		LOG_E("Invalid args");
		return NULL;
	}

	int socket_fd = -1;
	struct sockaddr_can addr;
	struct ifreq ifr;
	int ret;
	can_handle handle = NULL;

	// 配置并启用 CAN 接口
	if (!setup_can_interface(config)) {
		LOG_E("CAN interface setup failed");
		return NULL;
	}

	// 创建CAN套接字
	socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (socket_fd < 0) {
		LOG_E("Create CAN socket failed: %s", strerror(errno));
		shutdown_can_interface(config);
		return NULL;
	}

	// 设置CAN设备名
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, config->can_dev_name, IFNAMSIZ - 1);

	// 获取CAN设备的接口索引
	ret = ioctl(socket_fd, SIOCGIFINDEX, &ifr);
	if (ret < 0) {
		LOG_E("Get CAN interface index failed: %s", strerror(errno));
		cleanup(NULL, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	// 设置CAN套接字地址
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	// 绑定套接字到CAN设备
	ret = bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		LOG_E("Bind CAN socket failed: %s", strerror(errno));
		cleanup(NULL, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	// 设置非阻塞模式
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		LOG_E("Set CAN socket to non-blocking mode failed: %s", strerror(errno));
		cleanup(NULL, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	// 配置过滤器
	if (!configure_can_filters(socket_fd, config->dev_filters, config->filter_num)) {
		cleanup(NULL, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	// 分配CAN设备结构体
	handle = calloc(1, sizeof(struct can_device));
	if (!handle) {
		LOG_E("Memory allocation failed for CAN device");
		cleanup(NULL, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}
	handle->config = config;
	handle->socket_fd = socket_fd;

	// 创建停止信号文件描述符
	handle->stop_fd = eventfd(0, EFD_NONBLOCK);
	if (handle->stop_fd < 0) {
		LOG_E("Create stop_fd failed: %s", strerror(errno));
		cleanup(handle, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	// 创建epoll实例
	handle->epfd = epoll_create1(0);
	if (handle->epfd < 0) {
		LOG_E("Create epoll failed: %s", strerror(errno));
		cleanup(handle, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	// 添加CAN套接字到epoll
	struct epoll_event ev = { .events = EPOLLIN, .data.fd = handle->socket_fd };
	if (epoll_ctl(handle->epfd, EPOLL_CTL_ADD, handle->socket_fd, &ev) < 0) {
		LOG_E("Add CAN socket to epoll failed: %s", strerror(errno));
		cleanup(handle, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	// 添加停止信号到epoll
	struct epoll_event stop_event = { .events = EPOLLIN, .data.fd = handle->stop_fd };
	if (epoll_ctl(handle->epfd, EPOLL_CTL_ADD, handle->stop_fd, &stop_event) < 0) {
		LOG_E("Add stop_fd to epoll failed: %s", strerror(errno));
		cleanup(handle, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	// 创建读取线程
	ret = pthread_create(&handle->thread, NULL, can_read_thread_func, handle);
	if (ret != 0) {
		LOG_E("Create read thread failed: %s", strerror(ret));
		cleanup(handle, socket_fd);
		shutdown_can_interface(config);
		return NULL;
	}

	return handle;
}

/**
 * @brief 关闭CAN设备
 *
 * 关闭CAN套接字,停止读取线程,释放所有分配的资源.
 *
 * @param handle CAN句柄
 */
void can_device_close(can_handle handle)
{
	if (!handle)
		return;

	// 发送停止信号
	uint64_t u = 1;
	if (write(handle->stop_fd, &u, sizeof(uint64_t)) < 0 && errno != EAGAIN)
		LOG_E("Write stop signal failed: %s", strerror(errno));

	pthread_join(handle->thread, NULL);
	close(handle->epfd);
	close(handle->stop_fd);
	close(handle->socket_fd);

	// 关闭CAN接口
	shutdown_can_interface(handle->config);

	free(handle);
}

/**
 * @brief CAN发送函数
 *
 * @param handle can 句柄
 * @param data 数据缓冲
 * @param len 数据长度
 * @return true 发送成功
 * @return false 发送失败
 */
bool can_device_write(can_handle handle, uint8_t *data, size_t len)
{
	if (!handle || !data || len == 0) {
		LOG_E("Invalid parameters: g_can_dev=%p, data=%p, len=%zu", handle, data, len);
		return false;
	}

	struct can_frame frame;
	size_t remaining = len;
	size_t offset = 0;

	while (remaining > 0) {
		size_t chunk_size = remaining > CAN_MAX_DLEN ? CAN_MAX_DLEN : remaining;

		if (!make_one_can_frame(handle, &frame, data + offset, chunk_size)) {
			LOG_E("Failed to construct CAN frame");
			return false;
		}

		int nbytes = write(handle->socket_fd, &frame, sizeof(struct can_frame));
		if (nbytes != sizeof(struct can_frame)) {
			LOG_E("Failed to write CAN frame: %s", strerror(errno));
			return false;
		}

		remaining -= chunk_size;
		offset += chunk_size;
	}

	return true;
}
