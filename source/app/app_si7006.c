/**
 * @file app_si7006.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 温湿度采集
 * @version 1.0
 * @date 2024-12-06
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
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

#include "app/app_si7006.h"
#include "utils/logger.h"

#define SI7006_FILE_PATH ("/dev/si7006")

enum si7006_cmd {
	MEASURE_RELATIVE_HUMIDITY = 0, // 测量湿度
	MEASURE_TEMEPERATURE,		   // 测量温度
	CMD_MAX,
};

struct si7006_task {
	int fd;				   // 文件描述符
	float humidity;		   // 转换后的湿度 (%RH)
	float temperature;	   // 转换后的温度 (°C)
	pthread_mutex_t mutex; // 互斥锁
	bool run_flag;		   // 运行标志
};

static struct si7006_task si7006 = {
	.fd = -1,
	.humidity = 0.0f,
	.temperature = 0.0f,
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.run_flag = true,
};

static void app_si7006_init(void)
{
	static bool inited = false;
	if (inited)
		return;
	inited = true;

	si7006.fd = open(SI7006_FILE_PATH, O_RDWR);
	if (si7006.fd < 0) {
		LOG_E("Can't open device %s", SI7006_FILE_PATH);
	}
}

static void app_si7006_deinit(void)
{
	if (si7006.fd > 0)
		close(si7006.fd);
}

static void collect_humi_temp(void)
{
	if (si7006.fd < 0)
		return;

	uint8_t mea_cmd1 = MEASURE_RELATIVE_HUMIDITY;
	uint8_t mea_cmd2 = MEASURE_TEMEPERATURE;

	// 发送测量湿度指令
	int num = write(si7006.fd, &mea_cmd1, 1);
	if (num != 1) {
		LOG_E("write cmd failed, err is:%d", num);
		return;
	}

	// 发送测量温度指令
	num = write(si7006.fd, &mea_cmd2, 1);
	if (num != 1) {
		LOG_E("write cmd failed, err is:%d", num);
		return;
	}

	uint8_t read_data[4] = { 0 }; // 读取4字节数据
	num = read(si7006.fd, read_data, 4);
	if (num != 4) {
		LOG_E("read data faied err is :%d", num);
		return;
	}

	// 原始数据
	uint16_t raw_humidity = (read_data[0] << 8) | read_data[1];
	uint16_t raw_temperature = (read_data[2] << 8) | read_data[3];

	// 公式转换
	float actual_humidity = (125.0f * raw_humidity / 65536.0f) - 6.0f;
	float actual_temperature = (175.72f * raw_temperature / 65536.0f) - 46.85f;

	pthread_mutex_lock(&si7006.mutex);
	si7006.humidity = actual_humidity;
	si7006.temperature = actual_temperature;
	pthread_mutex_unlock(&si7006.mutex);

	LOG_I("humidity is %.2f %%RH, temperature is %.2f °C", actual_humidity, actual_temperature);
}

/**********************API**********************/

#define SI7006_TASK_PRIOD_MS (1000) // 1秒

/**
 * @brief 采集温湿度任务
 * 
 * @param arg 线程参数
 * @return void* 返回 NULL
 */
void *app_si7006_task(void *arg)
{
	app_si7006_init();

	struct timespec req_initial;
	req_initial.tv_sec = SI7006_TASK_PRIOD_MS / 1000;
	req_initial.tv_nsec = (SI7006_TASK_PRIOD_MS % 1000) * 1000000;

	while (si7006.run_flag) {
		collect_humi_temp();

		struct timespec req = req_initial;
		struct timespec remain;

		while (1) {
			int ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &req, &remain);
			if (ret == 0)
				break;
			else if (ret == EINTR) {
				req = remain;
			} else {
				LOG_E("clock_nanosleep failed: %s", strerror(ret));
				break;
			}
		}
	}

	app_si7006_deinit();

	return NULL;
}

/**
 * @brief 停止采集任务
 * 
 */
void app_si7006_task_stop(void)
{
	si7006.run_flag = false;
}

/**
 * @brief 获取湿度
 * 
 * @return float 湿度值 (%RH)
 */
float get_humidity(void)
{
	float humidity = 0.0f;
	pthread_mutex_lock(&si7006.mutex);
	humidity = si7006.humidity;
	pthread_mutex_unlock(&si7006.mutex);
	return humidity;
}

/**
 * @brief 获取温度
 * 
 * @return float 温度值 (°C)
 */
float get_temperature(void)
{
	float temperature = 0.0f;
	pthread_mutex_lock(&si7006.mutex);
	temperature = si7006.temperature;
	pthread_mutex_unlock(&si7006.mutex);
	return temperature;
}
