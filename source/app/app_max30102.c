#include <stdio.h>
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

/**
 * 将数据读取与计算逻辑分离，将计算放到独立线程中。
 * 去除过多的全局变量，使用一个上下文结构体统一管理状态。
 * 
 * 改动点：
 * 1. 增加 max30102_context_t 结构体，用于存放所有原先的全局状态数据。
 * 2. 在初始化函数中创建线程，用于持续从设备读取数据，并定期调用算法计算心率和血氧。
 * 3. 原先在 init 和 task 函数中的计算移到线程中进行。
 * 4. app_max30102_task 中不再重复从设备读取数据，只需要从共享数据中获取当前结果或执行展示逻辑。
 * 5. 去初始化时停止线程，确保安全退出。
 */

#define MAX30102_FILE_PATH "/dev/iio:device2" // 用于数据读取的设备节点
#define SYSFS_DEVICE_BASE_PATH "/sys/bus/iio/devices/iio:device2"
#define RED_ENABLE_PATH SYSFS_DEVICE_BASE_PATH "/scan_elements/in_intensity_red_en"
#define IR_ENABLE_PATH SYSFS_DEVICE_BASE_PATH "/scan_elements/in_intensity_ir_en"
#define BUFFER_ENABLE_PATH SYSFS_DEVICE_BASE_PATH "/buffer/enable"
#define BUFFER_LENGTH_PATH SYSFS_DEVICE_BASE_PATH "/buffer/length"

#define DATA_MASK ((1 << 18) - 1)
#define DATA_SHIFT 8
#define DATA_BUF_SIZE 400
#define SAMPLES_PER_CYCLE 100

typedef struct {
	int fd;
	int spo2_valid;
	int SpO2;
	int hr_valid;
	int heart_rate;

	unsigned int aun_red_buf[DATA_BUF_SIZE];
	unsigned int aun_ir_buf[DATA_BUF_SIZE];

	pthread_t calc_thread;
	pthread_mutex_t data_lock;
	bool thread_running;
} max30102_context_t;

static max30102_context_t ctx = {
	.fd = -1,
	.spo2_valid = 0,
	.SpO2 = 0,
	.hr_valid = 0,
	.heart_rate = 0,
	.thread_running = false,
};

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

static int enable_disable_buffer(int enable)
{
	return write_sys_int(BUFFER_ENABLE_PATH, enable);
}

static int set_buffer_len(int len)
{
	return write_sys_int(BUFFER_LENGTH_PATH, len);
}

static void cleanup(void)
{
	if (ctx.fd >= 0) {
		close(ctx.fd);
		ctx.fd = -1;
	}
	enable_disable_buffer(0);
	enable_disable_all_channels(0);
}

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

static void shift_data_buffer(
	unsigned int *red_buf, unsigned int *ir_buf, int total_len, int shift_len)
{
	for (int i = shift_len; i < total_len; i++) {
		red_buf[i - shift_len] = red_buf[i];
		ir_buf[i - shift_len] = ir_buf[i];
	}
}

static void *calc_thread_func(void *arg)
{
	// 首次读取DATA_BUF_SIZE个样本初始化
	for (int j = 0; j < DATA_BUF_SIZE; j++) {
		if (!read_one_sample(ctx.fd, &ctx.aun_red_buf[j], &ctx.aun_ir_buf[j])) {
			// 如果读取失败，可根据需要重试或退出
			j--;
		}
	}

	// 初次计算
	pthread_mutex_lock(&ctx.data_lock);
	maxim_heart_rate_and_oxygen_saturation(ctx.aun_ir_buf, DATA_BUF_SIZE, ctx.aun_red_buf,
		&ctx.SpO2, &ctx.spo2_valid, &ctx.heart_rate, &ctx.hr_valid);
	pthread_mutex_unlock(&ctx.data_lock);

	// 按照原逻辑：每次读取100新数据, 移动缓冲区, 再计算
	while (ctx.thread_running) {
		// 移动数据
		shift_data_buffer(ctx.aun_red_buf, ctx.aun_ir_buf, DATA_BUF_SIZE, SAMPLES_PER_CYCLE);

		// 读入100个新数据
		for (int j = DATA_BUF_SIZE - SAMPLES_PER_CYCLE; j < DATA_BUF_SIZE; j++) {
			if (!ctx.thread_running)
				break;

			if (!read_one_sample(ctx.fd, &ctx.aun_red_buf[j], &ctx.aun_ir_buf[j])) {
				j--;
			}
		}

		// 再次计算
		pthread_mutex_lock(&ctx.data_lock);
		maxim_heart_rate_and_oxygen_saturation(ctx.aun_ir_buf, DATA_BUF_SIZE, ctx.aun_red_buf,
			&ctx.SpO2, &ctx.spo2_valid, &ctx.heart_rate, &ctx.hr_valid);
		pthread_mutex_unlock(&ctx.data_lock);
	}
	return NULL;
}

/**
 * @brief 初始化函数
 * @return true 初始化成功，否则false
 */
bool app_max30102_init(void)
{
	if (enable_disable_all_channels(1) < 0)
		return false;

	if (set_buffer_len(4) < 0) {
		enable_disable_all_channels(0);
		return false;
	}

	if (enable_disable_buffer(1) < 0) {
		enable_disable_all_channels(0);
		return false;
	}

	ctx.fd = open(MAX30102_FILE_PATH, O_RDONLY | O_NONBLOCK);
	if (ctx.fd < 0) {
		LOG_E("Failed open device: %s\n", strerror(errno));
		cleanup();
		return false;
	}

	pthread_mutex_init(&ctx.data_lock, NULL);
	ctx.thread_running = true;
	if (pthread_create(&ctx.calc_thread, NULL, calc_thread_func, NULL) != 0) {
		LOG_E("Failed to create calc thread\n");
		ctx.thread_running = false;
		cleanup();
		pthread_mutex_destroy(&ctx.data_lock);
		return false;
	}

	return true;
}

/**
 * @brief 周期性任务函数：现在不在这里计算，只输出或使用最新的计算结果。
 */
void app_max30102_task(void)
{
	pthread_mutex_lock(&ctx.data_lock);
	if (ctx.spo2_valid) {
		get_sensor_data()->max30102.blood_oxygen = ctx.SpO2;
		LOG_I("SpO2: %d\t", ctx.SpO2);
	}

	if (ctx.hr_valid) {
		get_sensor_data()->max30102.heart_rate = ctx.heart_rate;
		LOG_I("heart rate: %d\n", ctx.heart_rate);
	}

	pthread_mutex_unlock(&ctx.data_lock);
}

/**
 * @brief 去初始化函数：释放资源
 */
void app_max30102_deinit(void)
{
	ctx.thread_running = false;
	pthread_join(ctx.calc_thread, NULL);
	pthread_mutex_destroy(&ctx.data_lock);
	cleanup();
}

/**
 * @brief 获取当前 SpO2 值
 */
int get_SpO2(void)
{
	pthread_mutex_lock(&ctx.data_lock);
	int val = ctx.SpO2;
	pthread_mutex_unlock(&ctx.data_lock);
	return val;
}

/**
 * @brief 获取当前心率值
 */
int get_heart_rate(void)
{
	pthread_mutex_lock(&ctx.data_lock);
	int val = ctx.heart_rate;
	pthread_mutex_unlock(&ctx.data_lock);
	return val;
}
