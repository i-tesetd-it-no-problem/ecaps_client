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
	uint16_t show_value;
};

static struct digital_device dig_dev = {
	.fd = -1,
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.show_value = 0,
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
	0x5E, // D
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

	pthread_mutex_lock(&dig_dev.mutex);
	uint16_t current_value = dig_dev.show_value;
	pthread_mutex_unlock(&dig_dev.mutex);

	for (int j = 0; j < 4; j++) {
		uint8_t tmp = (current_value >> (12 - 4 * j)) & 0x0F;
		buf[2 * j] = 1 << j;
		buf[2 * j + 1] = get_map_value(tmp);
	}

	return true;
}

bool app_digital_init(void)
{
	dig_dev.fd = open(DEV_PATH, O_WRONLY);
	if (dig_dev.fd < 0)
		return false;
	else
		return true;
}

void app_digital_deinit(void)
{
	if (dig_dev.fd >= 0) {
		close(dig_dev.fd);
		dig_dev.fd = -1;
		LOG_I("Digital device closed.");
	}
}

static void digital_test(void)
{
	static size_t counter = 0;
	counter += APP_DIGITAL_TASK_PERIOD;
	if (counter >= 1000) {
		counter = 0;
		dig_dev.show_value++;
	}
}

void app_digital_task(void)
{
	if (dig_dev.fd < 0)
		return;

	digital_test();

	uint8_t buf[DIGITAL_BYTES] = { 0 };
	bool ret = get_trans_data(dig_dev.show_value, buf, sizeof(buf));
	if (!ret)
		return;

	for (int i = 0; i < 4; i++) {
		ssize_t num = write(dig_dev.fd, buf + i * 2, 2);
		if (num != 2) {
			LOG_E("Write data failed, num is %zd", num);
			close(dig_dev.fd);
			dig_dev.fd = -1;
			return;
		}
	}
}

void app_change_digital(uint16_t value)
{
	pthread_mutex_lock(&dig_dev.mutex);
	dig_dev.show_value = value;
	pthread_mutex_unlock(&dig_dev.mutex);
}

uint16_t app_get_digital_value(void)
{
	uint16_t value;
	pthread_mutex_lock(&dig_dev.mutex);
	value = dig_dev.show_value;
	pthread_mutex_unlock(&dig_dev.mutex);
	return value;
}
