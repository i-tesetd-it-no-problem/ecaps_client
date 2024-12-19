/**
 * @file app_can_task.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief CAN 测试代码
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

#include <stdbool.h>
#include <stdint.h>

#include "utils/logger.h"

#include "app/can_device.h"
#include "app/app_can_task.h"

// CAN 报文处理
static void app_can_frame_handle(struct can_frame *frame);

// 允许接受的CAN ID
static struct can_filter filts[] = {
	{
		.can_id = 0x1B0, // 测试 0x1B0 就是本身
		.can_mask = CAN_SFF_MASK,
	},
	{
		.can_id = 0x1B1,
		.can_mask = CAN_SFF_MASK,
	},
	{
		.can_id = 0x1B2,
		.can_mask = CAN_SFF_MASK,
	},
};

// CAN设备配置
static const struct can_config config = {
	.can_bitrate = 250000,
	.can_dev_name = "can0",
	.can_id = 0x1B0,
	.dev_filters = filts,
	.filter_num = sizeof(filts) / sizeof(struct can_filter),
	.cb = app_can_frame_handle,
};

// CAN 报文处理
static void app_can_frame_handle(struct can_frame *frame)
{
	if (!frame) {
		LOG_E("Invalid frame");
		return;
	}

	LOG_I("Received CAN frame: ID=0x%x DLC=%d Data=[%02x %02x %02x %02x %02x %02x %02x %02x]",
		frame->can_id, frame->can_dlc, frame->data[0], frame->data[1], frame->data[2],
		frame->data[3], frame->data[4], frame->data[5], frame->data[6], frame->data[7]);
}

/**
 * @brief CAN任务初始化
 * 
 * @param priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_can_init(void **priv)
{
	if (!priv)
		return false;

	can_handle handle;

	handle = can_device_init(&config); // 初始化CAN设备
	if (!handle) {
		LOG_E("Can device init failed");
		return false;
	}

	*priv = handle;

	return true;
}

/**
 * @brief CAN 设备去初始化
 * 
 * @param priv 私有数据
 */
void app_can_deinit(void *priv)
{
	if (!priv)
		return;

	can_handle handle = priv;

	can_device_close(handle); // 关闭CAN设备
}

/**
 * @brief CAN测试函数
 */
static void app_can_test(can_handle handle)
{
	if (!handle) {
		return;
	}

	static size_t counter = 0;
	counter += APP_CAN_TASK_PERIOD;
	if (counter < 1000)
		return;
	counter = 0;

	uint8_t cmd = 16;		 // 指令 亮指示灯
	uint8_t frame_index = 0; // 第几帧
	uint8_t len_high = 0;	 // 数据长度 高字节
	uint8_t len_low = 2;	 // 数据长度 低字节

	uint8_t send_data[] = { cmd, frame_index, len_high, len_low, 0xFF, 0x01 }; // 全亮

	bool ret = can_device_write(handle, send_data, sizeof(send_data));
	if (!ret)
		LOG_E("Can test Failed");
	else
		LOG_I("Can send success");
}

/**
 * @brief CAN 任务
 * 
 * @param priv 私有数据
 */
void app_can_task(void *priv)
{
	if (!priv)
		return;

	can_handle handle = priv;

	app_can_test(handle);
}
