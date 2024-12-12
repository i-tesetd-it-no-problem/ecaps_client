/**
 * @file app_max30102.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 心率血氧采集模块
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <endian.h>
#include <stdbool.h>
#include <pthread.h>

#include "json/sensor_json.h"

#include "utils/logger.h"

#include "app/algorithm.h"
#include "app/app_max30102.h"

// MAX30102 设备节点路径
#define MAX30102_FILE_PATH "/dev/iio:device2"

// sysfs 中的设备基路径
#define SYSFS_DEVICE_BASE_PATH "/sys/bus/iio/devices/iio:device2"

// 红光传感器路径
#define RED_ENABLE_PATH SYSFS_DEVICE_BASE_PATH "/scan_elements/in_intensity_red_en"

// 红外传感器启用路径
#define IR_ENABLE_PATH SYSFS_DEVICE_BASE_PATH "/scan_elements/in_intensity_ir_en"

// 缓冲区使能路径
#define BUFFER_ENABLE_PATH SYSFS_DEVICE_BASE_PATH "/buffer/enable"

// 缓冲区长度设置路径
#define BUFFER_LENGTH_PATH SYSFS_DEVICE_BASE_PATH "/buffer/length"

#define DATA_MASK ((1 << 18) - 1)
#define DATA_SHIFT 8
#define DATA_BUF_SIZE 400
#define SAMPLES_PER_CYCLE 100

// MAX30102 任务结构体
struct max30102_task {
	int fd;			// 设备文件描述符
	int spo2_valid; // SpO2 数据有效性标志 1 有效,0 无效
	int SpO2;		// SpO2 数据值,血氧饱和度百分比,范围通常是 0 到 100
	int hr_valid;	// 心率数据有效性标志,1 有效,0 无效
	int heart_rate; // 心率数据值,单位 BPM(每分钟心跳次数)
	unsigned int aun_red_buf[DATA_BUF_SIZE]; // 红光传感器的数据缓冲区
	unsigned int aun_ir_buf[DATA_BUF_SIZE];	 // 红外传感器的数据缓冲区
	pthread_t calc_thread;					 //计算线程
	pthread_mutex_t data_lock;				 // 互斥锁
	bool thread_running;					 // 线程运行标志
};

/**
 * @brief 向指定的文件写入整数数据
 * 
 * 该函数将整数数据写入指定的文件,通常用于向设备节点或系统文件中写入配置或控制命令.
 * 例如,写入传感器的启用状态、配置值或其他整数参数.
 * 
 * @param filename 文件路径,指定目标文件的位置
 * @param data 要写入的数据,通常是一个整数
 * @return int 返回值：
 *         - 如果成功写入数据,返回 0.
 *         - 如果发生错误,返回负值,并设置 errno 以指示具体错误.
 */
static int write_sys_int(const char *filename, int data)
{
	int ret = 0;
	int file_fd = open(filename, O_WRONLY);
	if (file_fd < 0) {
		ret = -errno;
		LOG_E("failed to open %s: %s\n", filename, strerror(errno));
		return ret;
	}

	char buf[16];
	int len = snprintf(buf, sizeof(buf), "%d", data);
	if (len < 0 || len >= (int)sizeof(buf)) {
		LOG_E("snprintf error or buffer overflow when formatting data for %s\n", filename);
		close(file_fd);
		return -EINVAL;
	}

	ssize_t written = write(file_fd, buf, len);
	if (written < 0) {
		LOG_E("Failed to write to %s: %s\n", filename, strerror(errno));
		ret = -errno;
		close(file_fd);
		return ret;
	} else if (written != len) {
		LOG_E("Partial write to %s: expected %d bytes, wrote %zd bytes\n", filename, len, written);
		close(file_fd);
		return -EIO;
	}

	if (close(file_fd) < 0) {
		ret = -errno;
		LOG_E("Failed to close %s: %s\n", filename, strerror(errno));
		return ret;
	}

	return 0;
}

/**
 * @brief 启用或禁用所有传感器通道
 * 
 * 该函数用于启用或禁用 MAX30102 传感器的红光和红外光传感器通道.
 * 通常用于在开始采集数据之前启用传感器,或者在结束采集时禁用传感器.
 * 
 * @param enable 如果为 1,则启用通道；如果为 0,则禁用通道
 * @return int 返回值：
 *         - 0 表示操作成功.
 *         - 负值表示操作失败.
 */
static int enable_disable_all_channels(int enable)
{
	int ret = 0;
	ret = write_sys_int(RED_ENABLE_PATH, enable);
	if (ret < 0)
		return ret;

	ret = write_sys_int(IR_ENABLE_PATH, enable);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * @brief 启用或禁用缓冲区
 * 
 * 该函数用于启用或禁用 MAX30102 设备的缓冲区.启用缓冲区是采集数据时的前提.
 * 
 * @param enable 如果为 1,则启用缓冲区；如果为 0,则禁用缓冲区
 * @return int 返回值：
 *         - 0 表示操作成功.
 *         - 负值表示操作失败.
 */
static int enable_disable_buffer(int enable)
{
	return write_sys_int(BUFFER_ENABLE_PATH, enable);
}

/**
 * @brief 设置缓冲区长度
 * 
 * 该函数用于设置 MAX30102 设备的缓冲区长度.缓冲区用于存储采集到的数据, 
 * 在数据采集过程中,根据需要动态调整缓冲区的大小.
 * 
 * @param len 缓冲区的长度,单位是样本数量
 * @return int 返回值：
 *         - 0 表示操作成功.
 *         - 负值表示操作失败.
 */
static int set_buffer_len(int len)
{
	return write_sys_int(BUFFER_LENGTH_PATH, len);
}

/**
 * @brief 清理和关闭 MAX30102 相关资源
 * 
 * 该函数用于关闭设备文件、禁用缓冲区和所有传感器通道,并释放相关资源.
 * 通常在任务结束或出现错误时调用.
 * 
 * @param max30102 指向 MAX30102 任务结构体的指针
 */
static void cleanup(struct max30102_task *max30102)
{
	if (max30102->fd >= 0) {
		close(max30102->fd);
		max30102->fd = -1;
	}
	enable_disable_buffer(0);
	enable_disable_all_channels(0);
}

/**
 * @brief 读取一个数据样本
 * 
 * 该函数用于从 MAX30102 设备中读取一个数据样本(包含红光和红外光的原始数据).
 * 读取的结果会通过参数传出.
 * 
 * @param fd 设备文件描述符
 * @param red_val 读取到的红光数据值
 * @param ir_val 读取到的红外光数据值
 * @return true 表示读取成功,false 表示读取失败
 */
static bool read_one_sample(int fd, unsigned int *red_val, unsigned int *ir_val)
{
	struct pollfd pfd = {
		.fd = fd,
		.events = POLLIN,
	};

	int ret = poll(&pfd, 1, -1);
	if (ret < 0) {
		LOG_E("Poll error: %s\n", strerror(errno));
		return false;
	} else if (ret == 0) {
		return false;
	}

	int data[2];
	int read_size = read(fd, data, 8);
	if (read_size < 0) {
		if (errno == EAGAIN) {
			LOG_E("nothing available\n");
			return false;
		} else {
			LOG_E("Read error: %s\n", strerror(errno));
			return false;
		}
	}

	int tmp_val = be32toh(data[0]);
	*red_val = (tmp_val >> DATA_SHIFT) & DATA_MASK;

	tmp_val = be32toh(data[1]);
	*ir_val = (tmp_val >> DATA_SHIFT) & DATA_MASK;

	return true;
}

/**
 * @brief 移动数据缓冲区
 * 
 * 该函数用于将已有的数据缓冲区向前移动指定的长度,并为新的数据腾出空间.
 * 通常用于滑动窗口技术中,以便不断更新数据缓冲区.
 * 
 * @param red_buf 红光数据缓冲区
 * @param ir_buf 红外光数据缓冲区
 * @param total_len 缓冲区的总长度
 * @param shift_len 移动的长度
 */
static void shift_data_buffer(
	unsigned int *red_buf, unsigned int *ir_buf, int total_len, int shift_len)
{
	for (int i = shift_len; i < total_len; i++) {
		red_buf[i - shift_len] = red_buf[i];
		ir_buf[i - shift_len] = ir_buf[i];
	}
}

/**
 * @brief 计算线程函数
 * 
 * 该函数在独立的线程中执行,定期读取样本数据并进行心率与血氧饱和度的计算.
 * 每读取一定数量的新数据后,会调用算法函数重新计算心率和血氧.
 * 
 * @param arg 传入的参数,通常是 MAX30102 任务结构体的指针
 * @return NULL
 */
static void *calc_thread_func(void *arg)
{
	struct max30102_task *max30102 = arg;

	// 首次读取DATA_BUF_SIZE个样本初始化
	for (int j = 0; j < DATA_BUF_SIZE; j++) {
		if (!read_one_sample(max30102->fd, &max30102->aun_red_buf[j], &max30102->aun_ir_buf[j])) {
			// 如果读取失败,可根据需要重试或退出
			j--;
		}
	}

	// 初次计算
	pthread_mutex_lock(&max30102->data_lock);
	maxim_heart_rate_and_oxygen_saturation(max30102->aun_ir_buf, DATA_BUF_SIZE,
		max30102->aun_red_buf, &max30102->SpO2, &max30102->spo2_valid, &max30102->heart_rate,
		&max30102->hr_valid);
	pthread_mutex_unlock(&max30102->data_lock);

	// 按照原逻辑：每次读取100新数据, 移动缓冲区, 再计算
	while (max30102->thread_running) {
		// 移动数据
		shift_data_buffer(
			max30102->aun_red_buf, max30102->aun_ir_buf, DATA_BUF_SIZE, SAMPLES_PER_CYCLE);

		// 读入100个新数据
		for (int j = DATA_BUF_SIZE - SAMPLES_PER_CYCLE; j < DATA_BUF_SIZE; j++) {
			if (!max30102->thread_running)
				break;

			if (!read_one_sample(
					max30102->fd, &max30102->aun_red_buf[j], &max30102->aun_ir_buf[j])) {
				j--;
			}
		}

		// 再次计算
		pthread_mutex_lock(&max30102->data_lock);
		maxim_heart_rate_and_oxygen_saturation(max30102->aun_ir_buf, DATA_BUF_SIZE,
			max30102->aun_red_buf, &max30102->SpO2, &max30102->spo2_valid, &max30102->heart_rate,
			&max30102->hr_valid);
		pthread_mutex_unlock(&max30102->data_lock);
	}
	return NULL;
}

/**
 * @brief 初始化采集模块
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_max30102_init(void **p_priv)
{
	if (!p_priv)
		return false;

	struct max30102_task *max30102 = malloc(sizeof(struct max30102_task));
	if (!max30102) {
		LOG_E("Malloc max30102 failed");
		return false;
	}
	memset(max30102, 0, sizeof(struct max30102_task));

	if (enable_disable_all_channels(1) < 0)
		goto err_free_max30102;

	if (set_buffer_len(4) < 0) {
		enable_disable_all_channels(0);
		goto err_free_max30102;
	}

	if (enable_disable_buffer(1) < 0) {
		enable_disable_all_channels(0);
		goto err_free_max30102;
	}

	max30102->fd = open(MAX30102_FILE_PATH, O_RDONLY | O_NONBLOCK);
	if (max30102->fd < 0) {
		LOG_E("Failed open device: %s\n", strerror(errno));
		goto err_clen;
	}

	int ret = pthread_mutex_init(&max30102->data_lock, NULL);
	if (ret != 0) {
		LOG_E("Init mutex failed");
		goto err_close_fd;
	}

	max30102->thread_running = true;
	if (pthread_create(&max30102->calc_thread, NULL, calc_thread_func, max30102) != 0) {
		LOG_E("Failed to create calc thread\n");
		goto err_free_mutex;
	}

	*p_priv = max30102;
	return true;

err_free_mutex:
	pthread_mutex_destroy(&max30102->data_lock);

err_close_fd:
	if (max30102->fd > 0)
		close(max30102->fd);

err_clen:
	cleanup(max30102);

err_free_max30102:
	free(max30102);

	return false;
}

/**
 * @brief 采集任务
 * 
 * @param priv 私有数据指针
 */
void app_max30102_task(void *priv)
{
	struct max30102_task *max30102 = priv;

	pthread_mutex_lock(&max30102->data_lock);
	if (max30102->spo2_valid) {
		get_sensor_data()->max30102.blood_oxygen = max30102->SpO2;
		LOG_I("SpO2: %d\t", max30102->SpO2);
	}

	if (max30102->hr_valid) {
		get_sensor_data()->max30102.heart_rate = max30102->heart_rate;
		LOG_I("heart rate: %d\n", max30102->heart_rate);
	}

	pthread_mutex_unlock(&max30102->data_lock);
}

/**
 * @brief 去初始化采集模块
 * 
 * @param priv 私有数据指针
 */
void app_max30102_deinit(void *priv)
{
	struct max30102_task *max30102 = priv;

	max30102->thread_running = false;
	pthread_join(max30102->calc_thread, NULL);
	pthread_mutex_destroy(&max30102->data_lock);
	cleanup(max30102);
	free(priv);
}
