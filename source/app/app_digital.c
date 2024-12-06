#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "app/app_digital.h"
#include "utils/logger.h"

#define DEV_NAME "digital"
#define DEV_PATH "/dev/" DEV_NAME

#define DIGITAL_BYTES (8)

struct digital_device {
	int fd;
	pthread_mutex_t mutex;
	bool run_flag;
	uint16_t show_value;
};

static struct digital_device dig_dev = {
	.fd = -1,
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.run_flag = true,
	.show_value = 0x1234,
};

// 0 - 9 A - F
static uint8_t digita_map[] = {
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F, // 9
	0x77, // A
	0x7C, // b
	0x39, // C
	0x5E, // d
	0x79, // E
	0x71, // F
};

static uint8_t get_map_value(uint8_t input)
{
	return input > 15 ? digita_map[0] : digita_map[input];
}

static bool get_trans_data(uint16_t show_value, uint8_t *buf, uint8_t len)
{
	if (!buf || len != DIGITAL_BYTES) {
		LOG_E("Invalid argument");
		return false;
	}

	uint16_t current_value = dig_dev.show_value;

	for (int j = 0; j < 4; j++) {
		uint8_t tmp = (current_value >> (12 - 4 * j)) & 0x0F;
		buf[2 * j] = 1 << j;
		buf[2 * j + 1] = get_map_value(tmp);
	}

	return true;
}

static void app_digital_init(void)
{
	static bool inited = false;
	if (inited)
		return;
	inited = true;

	dig_dev.fd = open(DEV_PATH, O_WRONLY);
	if (dig_dev.fd < 0) {
		LOG_E("Failed to open device: %s", DEV_PATH);
	}
}

static void app_digital_deinit(void)
{
	if (dig_dev.fd >= 0) {
		close(dig_dev.fd);
		dig_dev.fd = -1;
	}
}

static void write_show_value(void)
{
	if (dig_dev.fd < 0)
		return;

	uint8_t buf[DIGITAL_BYTES] = { 0 };
	bool ret = get_trans_data(dig_dev.show_value, buf, sizeof(buf));
	if (!ret)
		return;

	for (int i = 0; i < 4; i++) {
		int num = write(dig_dev.fd, buf + i * 2, 2);
		if (num != 2) {
			LOG_E("Write data failed, num is %d", num);
			close(dig_dev.fd);
			dig_dev.fd = -1;
		}
	}
}

static void test(size_t ms)
{
	static size_t count = 0;
	count += ms;

	// 每秒递增一次
	if (count >= 1000) {
		dig_dev.show_value++;
		count = 0;
	}
}

/***********************API***********************/

// 周期1ms
#define PERIOD_MS (1)

void *app_digital_task(void *arg)
{
	app_digital_init();

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	while (dig_dev.run_flag) {
		write_show_value();
		test(PERIOD_MS); // 测试代码

		ts.tv_nsec += PERIOD_MS * 1000000;
		while (ts.tv_nsec >= 1000000000) {
			ts.tv_nsec -= 1000000000;
			ts.tv_sec++;
		}
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
	}

	app_digital_deinit();
	return NULL;
}

void app_change_digital(uint16_t value)
{
	pthread_mutex_lock(&dig_dev.mutex);
	dig_dev.show_value = value;
	pthread_mutex_unlock(&dig_dev.mutex);
}

void app_digital_stop(void)
{
	dig_dev.run_flag = false;
}
