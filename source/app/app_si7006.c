/**
 * @file app_si7006.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 温湿度采集模块
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

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "utils/logger.h"

#include "json/sensor_json.h"

#include "app/app_si7006.h"

#define SI7006_FILE_PATH ("/dev/si7006")

enum si7006_cmd {
	MEASURE_RELATIVE_HUMIDITY = 0, // 测量湿度
	MEASURE_TEMEPERATURE,		   // 测量温度
	CMD_MAX,
};

struct si7006_task {
	int fd; // 文件描述符
};

/**
 * @brief 初始化采集模块
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_si7006_init(void **p_priv)
{
	struct si7006_task *si7006 = malloc(sizeof(struct si7006_task));
	if (!si7006) {
		LOG_E("Malloc si7006 ffailed");
		return false;
	}
	memset(si7006, 0, sizeof(struct si7006_task));

	si7006->fd = open(SI7006_FILE_PATH, O_RDWR);
	if (si7006->fd < 0) {
		free(si7006);
		LOG_E("Can't open device %s", SI7006_FILE_PATH);
		return false;
	}

	*p_priv = si7006;

	return true;
}

/**
 * @brief 去初始化采集模块
 * 
 * @param priv 私有数据指针
 */
void app_si7006_deinit(void *priv)
{
	if (!priv)
		return;

	struct si7006_task *si7006 = priv;

	if (si7006->fd >= 0)
		close(si7006->fd);

	free(si7006);
}

/**
 * @brief 采集任务
 * 
 * @param priv 私有数据指针
 */
void app_si7006_task(void *priv)
{
	if (!priv)
		return;

	struct si7006_task *si7006 = priv;

	if (si7006->fd < 0) {
		LOG_E("SI7006 device not opened.");
		return;
	}

	uint8_t mea_cmd1 = MEASURE_RELATIVE_HUMIDITY;
	uint8_t mea_cmd2 = MEASURE_TEMEPERATURE;

	// 发送测量湿度指令
	ssize_t num = write(si7006->fd, &mea_cmd1, 1);
	if (num != 1) {
		LOG_E("Failed to write humidity measure command, err: %zd", num);
		return;
	}

	// 发送测量温度指令
	num = write(si7006->fd, &mea_cmd2, 1);
	if (num != 1) {
		LOG_E("Failed to write temperature measure command, err: %zd", num);
		return;
	}

	uint8_t read_data[4] = { 0 }; // 读取4字节数据
	num = read(si7006->fd, read_data, 4);
	if (num != 4) {
		LOG_E("Failed to read data, err: %zd", num);
		return;
	}

	// 原始数据
	uint16_t raw_humidity = (read_data[0] << 8) | read_data[1];
	uint16_t raw_temperature = (read_data[2] << 8) | read_data[3];

	// 公式转换
	float actual_humidity = (125.0f * raw_humidity / 65536.0f) - 6.0f;
	float actual_temperature = (175.72f * raw_temperature / 65536.0f) - 46.85f;

	LOG_I("Humidity: %.2f %%RH, Temperature: %.2f °C", actual_humidity, actual_temperature);

	get_sensor_data()->si7006.humidity = actual_humidity;
	get_sensor_data()->si7006.temperature = actual_temperature;

	return;
}
