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

#include "app/app_vol_cur.h"
#include "utils/logger.h"

#define R50 (1000.0)
#define R49 (100 * 1000.0)
#define R53 (10.0)
#define R54 (0.1)

#define IIO_DEVICE (2)
#define VOLTAGE_CHANNEL (18)
#define CURRENT_CHANNEL (19)

#define RAW_VOLTAGE_FORMAT "/sys/bus/iio/devices/iio:device%d/in_voltage%d_raw"
#define RAW_CURRENT_FORMAT "/sys/bus/iio/devices/iio:device%d/in_voltage%d_raw"
#define SCALE_FORMAT "/sys/bus/iio/devices/iio:device%d/in_voltage_scale"

struct coll_task {
	float voltage;		   // 电压
	float current;		   // 电流
	bool run_flag;		   // 运行标志
	pthread_rwlock_t lock; // 读写锁

	// 文件描述符
	int fd_raw_v;
	int fd_raw_i;
	int fd_scale;

	// 计算电压电流需要
	float scale; // 精度
};

// 初始化结构体实例
static struct coll_task instance = {
	.current = 0.0f,
	.voltage = 0.0f,
	.lock = PTHREAD_RWLOCK_INITIALIZER,
	.run_flag = true,

	.fd_raw_v = -1,
	.fd_raw_i = -1,
	.fd_scale = -1,
};

// 读取精度和偏移
static int read_scale(struct coll_task *inst)
{
	if (inst->fd_scale < 0)
		return -1;

	char buf[32];
	ssize_t ret;

	// 读取 scale
	memset(buf, 0, sizeof(buf));
	ret = read(inst->fd_scale, buf, sizeof(buf) - 1);
	if (ret < 0) {
		LOG_E("read scale failed: %s", strerror(errno));
		return -1;
	}
	char *endptr;
	float scale = strtof(buf, &endptr);
	if (endptr == buf) {
		LOG_E("Invalid scale value: %s", buf);
		return -1;
	}
	inst->scale = scale;
	if (lseek(inst->fd_scale, 0, SEEK_SET) < 0) {
		LOG_E("lseek failed for scale: %s", strerror(errno));
		return -1;
	}

	return 0;
}

// 读取原始值并计算电压
static int collect_voltage(struct coll_task *inst)
{
	if (inst->fd_raw_v < 0)
		return -1;

	// 每次采集前读取 scale 和 offset
	if (read_scale(inst) != 0) {
		LOG_E("Failed to read scale and offset");
		return -1;
	}

	char buf[32];
	ssize_t ret;

	memset(buf, 0, sizeof(buf));
	ret = read(inst->fd_raw_v, buf, sizeof(buf) - 1);
	if (ret < 0) {
		LOG_E("read raw voltage failed: %s", strerror(errno));
		return -1;
	}
	char *endptr;
	long raw_value = strtol(buf, &endptr, 10); // 读取原始值
	if (endptr == buf) {
		LOG_E("Invalid raw voltage value: %s", buf);
		return -1;
	}

	float raw_vol = raw_value * inst->scale; // 采集电压 mV

	if (pthread_rwlock_wrlock(&inst->lock) != 0) {
		LOG_E("Failed to lock");
		return -1;
	}
	inst->voltage = raw_vol;
	pthread_rwlock_unlock(&inst->lock);

	if (lseek(inst->fd_raw_v, 0, SEEK_SET) < 0) {
		LOG_E("lseek failed for raw voltage: %s", strerror(errno));
		return -1;
	}

	LOG_I("voltage is %.2fmV", inst->voltage);

	return 0;
}

// 读取原始值并计算电流
static int collect_current(struct coll_task *inst)
{
	if (inst->fd_raw_i < 0)
		return -1;

	// 每次采集前读取 scale
	if (read_scale(inst) != 0) {
		LOG_E("Failed to read scale and offset");
		return -1;
	}

	char buf[32];
	ssize_t ret;

	memset(buf, 0, sizeof(buf));
	ret = read(inst->fd_raw_i, buf, sizeof(buf) - 1);
	if (ret < 0) {
		LOG_E("read raw current failed: %s", strerror(errno));
		return -1;
	}
	char *endptr;
	long raw_value = strtol(buf, &endptr, 10); // 读取原始值
	if (endptr == buf) {
		LOG_E("Invalid raw current value: %s", buf);
		return -1;
	}

	float Vcoll = raw_value * inst->scale;		 // 采集电压 mV
	float Vin = Vcoll / (R50 + R49 + R53) * R50; // 输入电压
	float Iin = Vin / R54;						 // 电流

	if (pthread_rwlock_wrlock(&inst->lock) != 0) {
		LOG_E("Failed to lock");
		return -1;
	}
	inst->current = Iin;
	pthread_rwlock_unlock(&inst->lock);

	if (lseek(inst->fd_raw_i, 0, SEEK_SET) < 0) {
		LOG_E("lseek failed for raw current: %s", strerror(errno));
		return -1;
	}

	LOG_I("current is %.2fmA", inst->current);

	return 0;
}

// 采集电压和电流
static int collect_data(struct coll_task *inst)
{
	if (collect_voltage(inst) != 0) {
		LOG_E("Failed to collect voltage data");
		return -1;
	}

	if (collect_current(inst) != 0) {
		LOG_E("Failed to collect current data");
		return -1;
	}

	return 0;
}

// 初始化文件描述符
static int app_coll_i_v_init(void)
{
	int ret;
	char path_buf[64];

	// 打开 raw voltage 文件
	memset(path_buf, 0, sizeof(path_buf));
	snprintf(path_buf, sizeof(path_buf), RAW_VOLTAGE_FORMAT, IIO_DEVICE, VOLTAGE_CHANNEL);
	instance.fd_raw_v = open(path_buf, O_RDONLY);
	if (instance.fd_raw_v < 0) {
		LOG_E("Failed to open raw voltage file: %s, error: %s", path_buf, strerror(errno));
		goto error;
	}

	// 打开 raw current 文件
	memset(path_buf, 0, sizeof(path_buf));
	snprintf(path_buf, sizeof(path_buf), RAW_CURRENT_FORMAT, IIO_DEVICE, CURRENT_CHANNEL);
	instance.fd_raw_i = open(path_buf, O_RDONLY);
	if (instance.fd_raw_i < 0) {
		LOG_E("Failed to open raw current file: %s, error: %s", path_buf, strerror(errno));
		goto error;
	}

	// 打开 scale 文件
	memset(path_buf, 0, sizeof(path_buf));
	snprintf(path_buf, sizeof(path_buf), SCALE_FORMAT, IIO_DEVICE);
	instance.fd_scale = open(path_buf, O_RDONLY);
	if (instance.fd_scale < 0) {
		LOG_E("Failed to open scale file: %s, error: %s", path_buf, strerror(errno));
		goto error;
	}

	return 0;

error:
	if (instance.fd_raw_v >= 0) {
		close(instance.fd_raw_v);
		instance.fd_raw_v = -1;
	}
	if (instance.fd_raw_i >= 0) {
		close(instance.fd_raw_i);
		instance.fd_raw_i = -1;
	}
	if (instance.fd_scale >= 0) {
		close(instance.fd_scale);
		instance.fd_scale = -1;
	}
	return -1;
}

// 关闭文件描述符
static void app_coll_i_v_deinit(void)
{
	if (instance.fd_scale >= 0) {
		close(instance.fd_scale);
		instance.fd_scale = -1;
	}
	if (instance.fd_raw_i >= 0) {
		close(instance.fd_raw_i);
		instance.fd_raw_i = -1;
	}
	if (instance.fd_raw_v >= 0) {
		close(instance.fd_raw_v);
		instance.fd_raw_v = -1;
	}
}

/**********************API**********************/

// 采集电压和电流数据
void *coll_v_i_task(void *arg)
{
	if (app_coll_i_v_init() != 0) {
		LOG_E("Failed to initialize voltage and current collection");
		return NULL;
	}

	const size_t period_ms = 2000; // 50ms 周期

	struct timespec req;
	req.tv_sec = period_ms / 1000;
	req.tv_nsec = (period_ms % 1000) * 1000000L; // 纳秒

	while (1) {
		if (!instance.run_flag)
			break;

		if (collect_data(&instance) != 0)
			LOG_E("Failed to collect data");

		nanosleep(&req, NULL);
	}

	app_coll_i_v_deinit();
	return NULL;
}

// 获取电压
float get_voltage(void)
{
	float voltage = 0.0f;
	if (pthread_rwlock_rdlock(&instance.lock) != 0) {
		LOG_E("Failed to lock in get_voltage");
		return 0.0f;
	}
	voltage = instance.voltage;
	pthread_rwlock_unlock(&instance.lock);
	return voltage;
}

// 获取电流
float get_current(void)
{
	float current = 0.0f;
	if (pthread_rwlock_rdlock(&instance.lock) != 0) {
		LOG_E("Failed to lock in get_current");
		return 0.0f;
	}
	current = instance.current;
	pthread_rwlock_unlock(&instance.lock);
	return current;
}

// 停止数据采集
void stop_coll_task(void)
{
	pthread_rwlock_wrlock(&instance.lock);
	instance.run_flag = false;
	pthread_rwlock_unlock(&instance.lock);
}
