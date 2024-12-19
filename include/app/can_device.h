/**
 * @file can_device.h
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

#ifndef _CAN_DEVICE_H
#define _CAN_DEVICE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <linux/can.h>

/**
 * @brief CAN帧回调
 *
 * 每接收到一帧数据都会调用此回调, 用户自行决定如何处理
 * NULL 代表无效数据
 * 
 */
typedef void (*can_frame_recv_cb)(struct can_frame *frame);
struct can_config {
	char *can_dev_name;				// `ifconfig -a` 显示的CAN名, 如 can0
	uint32_t can_id;				// (11/29 bit)
	size_t can_bitrate;				// can 比特率
	struct can_filter *dev_filters; // 过滤器数组
	size_t filter_num;				// 过滤器长度
	can_frame_recv_cb cb;			// CAN报文 处理函数
};

typedef struct can_device *can_handle; // CAN 句柄

/**
 * @brief CAN设备初始化
 *
 * 初始化CAN套接字,设置接口,绑定套接字到CAN设备,创建epoll实例,等
 * 添加CAN套接字和eventfd到epoll
 * 成功后会启动读取线程
 * 每当读取成功一帧就会调用 can_frame_recv_cb 函数指针
 *
 * @param config 用户配置信息
 * @return can_handle 初始化成功
 * @return NULL 初始化失败
 */
can_handle can_device_init(const struct can_config *config);

/**
 * @brief 关闭CAN设备
 *
 * 关闭CAN套接字,停止读取线程,释放所有分配的资源.
 *
 * @param handle CAN句柄
 */
void can_device_close(can_handle handle);

/**
 * @brief CAN发送函数
 *
 * @param handle can 句柄
 * @param data 数据缓冲
 * @param len 数据长度
 * @return true 发送成功
 * @return false 发送失败
 */
bool can_device_write(can_handle handle, uint8_t *data, size_t len);

#endif /* _CAN_DEVICE_H */