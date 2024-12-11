/**
 * @file app_vol_cur.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 扩展版工作电压/电流采集
 * @version 1.0
 * @date 2024-12-04
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
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "json/sensor_json.h"

#include "utils/logger.h"

#include "app/app_lmv358.h"

#define R50 (1000.0)
#define R49 (100 * 1000.0)
#define R53 (10.0)
#define R54 (0.1)

#define IIO_DEVICE (3)
#define VOLTAGE_CHANNEL (18)
#define CURRENT_CHANNEL (19)

#define RAW_VOLTAGE_FORMAT "/sys/bus/iio/devices/iio:device%d/in_voltage%d_raw"
#define RAW_CURRENT_FORMAT "/sys/bus/iio/devices/iio:device%d/in_voltage%d_raw"
#define SCALE_FORMAT "/sys/bus/iio/devices/iio:device%d/in_voltage_scale"

struct lmv358_task {
	bool run_flag; // 运行标志

	// 文件描述符
	int fd_raw_v;
	int fd_raw_i;
	int fd_scale;

	float scale;
};

// 读取精度和偏移
static int read_scale(struct lmv358_task *lmv358)
{
	if (lmv358->fd_scale < 0)
		return -1;

	char buf[32];
	ssize_t ret;

	// 读取 scale
	memset(buf, 0, sizeof(buf));
	ret = read(lmv358->fd_scale, buf, sizeof(buf) - 1);
	if (ret < 0) {
		LOG_E("read scale failed");
		return -1;
	}
	char *endptr;
	float scale = strtof(buf, &endptr);
	if (endptr == buf) {
		LOG_E("Invalid scale value: %s", buf);
		return -1;
	}
	lmv358->scale = scale;
	if (lseek(lmv358->fd_scale, 0, SEEK_SET) < 0) {
		LOG_E("lseek failed for scale");
		return -1;
	}

	return 0;
}

// 读取原始值并计算电压
static int collect_voltage(struct lmv358_task *lmv358)
{
	if (lmv358->fd_raw_v < 0)
		return -1;

	// 每次采集前读取 scale
	if (read_scale(lmv358) != 0) {
		LOG_E("Failed to read scale");
		return -1;
	}

	char buf[32];
	ssize_t ret;

	memset(buf, 0, sizeof(buf));
	ret = read(lmv358->fd_raw_v, buf, sizeof(buf) - 1);
	if (ret < 0) {
		LOG_E("read raw voltage failed");
		return -1;
	}
	char *endptr;
	long raw_value = strtol(buf, &endptr, 10); // 读取原始值
	if (endptr == buf) {
		LOG_E("Invalid raw voltage value: %s", buf);
		return -1;
	}

	float raw_vol = raw_value * lmv358->scale; // 采集电压 mV

	if (lseek(lmv358->fd_raw_v, 0, SEEK_SET) < 0) {
		LOG_E("lseek failed for raw voltage");
		return -1;
	}

	get_sensor_data()->lmv358.voltage = raw_vol;

	LOG_I("voltage is %.2fmV", raw_vol);

	return 0;
}

// 读取原始值并计算电流
static int collect_current(struct lmv358_task *lmv358)
{
	if (lmv358->fd_raw_i < 0)
		return -1;

	// 每次采集前读取 scale
	if (read_scale(lmv358) != 0) {
		LOG_E("Failed to read scale");
		return -1;
	}

	char buf[32];
	ssize_t ret;

	memset(buf, 0, sizeof(buf));
	ret = read(lmv358->fd_raw_i, buf, sizeof(buf) - 1);
	if (ret < 0) {
		LOG_E("read raw current failed");
		return -1;
	}
	char *endptr;
	long raw_value = strtol(buf, &endptr, 10); // 读取原始值
	if (endptr == buf) {
		LOG_E("Invalid raw current value: %s", buf);
		return -1;
	}

	float Vcoll = raw_value * lmv358->scale;	 // 采集电压 mV
	float Vin = Vcoll / (R50 + R49 + R53) * R50; // 输入电压
	float Iin = Vin / R54;						 // 电流

	if (lseek(lmv358->fd_raw_i, 0, SEEK_SET) < 0) {
		LOG_E("lseek failed for raw current");
		return -1;
	}

	get_sensor_data()->lmv358.current = Iin;

	LOG_I("current is %.2fmA", Iin);

	return 0;
}

/**
 * @brief 初始化采集模块
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_lmv358_init(void **p_priv)
{
	int ret;
	char path_buf[64];

	// 初始化结构体实例
	struct lmv358_task *lmv358 = malloc(sizeof(struct lmv358_task));
	if (!lmv358) {
		LOG_E("Malloc lmv358 failed");
		return false;
	}
	lmv358->run_flag = true;

	// 打开 raw voltage 文件
	memset(path_buf, 0, sizeof(path_buf));
	snprintf(path_buf, sizeof(path_buf), RAW_VOLTAGE_FORMAT, IIO_DEVICE, VOLTAGE_CHANNEL);
	lmv358->fd_raw_v = open(path_buf, O_RDONLY);
	if (lmv358->fd_raw_v < 0) {
		LOG_E("Failed to open raw voltage file: %s", path_buf);
		goto free_lmv358;
	}

	// 打开 raw current 文件
	memset(path_buf, 0, sizeof(path_buf));
	snprintf(path_buf, sizeof(path_buf), RAW_CURRENT_FORMAT, IIO_DEVICE, CURRENT_CHANNEL);
	lmv358->fd_raw_i = open(path_buf, O_RDONLY);
	if (lmv358->fd_raw_i < 0) {
		LOG_E("Failed to open raw current file: %s", path_buf);
		goto err_close_vol;
	}

	// 打开 scale 文件
	memset(path_buf, 0, sizeof(path_buf));
	snprintf(path_buf, sizeof(path_buf), SCALE_FORMAT, IIO_DEVICE);
	lmv358->fd_scale = open(path_buf, O_RDONLY);
	if (lmv358->fd_scale < 0) {
		LOG_E("Failed to open scale file: %s", path_buf);
		goto err_close_i;
	}

	*p_priv = lmv358;

	return true;

err_close_i:
	close(lmv358->fd_raw_i);

err_close_vol:
	close(lmv358->fd_raw_v);

free_lmv358:
	free(lmv358);

	return false;
}

/**
 * @brief 去初始化采集模块
 * 
 * @param priv 私有数据指针
 */
void app_lmv358_deinit(void *priv)
{
	struct lmv358_task *lmv358 = priv;

	if (lmv358->fd_scale >= 0) {
		close(lmv358->fd_scale);
		lmv358->fd_scale = -1;
	}

	if (lmv358->fd_raw_i >= 0) {
		close(lmv358->fd_raw_i);
		lmv358->fd_raw_i = -1;
	}

	if (lmv358->fd_raw_v >= 0) {
		close(lmv358->fd_raw_v);
		lmv358->fd_raw_v = -1;
	}

	free(lmv358);
}

/**
 * @brief 采集任务
 * 
 * @param priv 私有数据指针
 */
void app_lmv358_task(void *priv)
{
	struct lmv358_task *lmv358 = priv;

	collect_voltage(lmv358);
	collect_current(lmv358);
}
