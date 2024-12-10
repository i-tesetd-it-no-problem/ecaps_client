#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "app/app_digital.h"
#include "utils/logger.h"

#define DEV_NAME "digital"
#define DEV_PATH "/dev/" DEV_NAME

#define DIGITAL_BYTES (8)

struct digital_dev {
	int fd;
	pthread_mutex_t mutex;
	uint16_t show_value;
};

struct digital_dev *digi_ins = NULL;

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

static bool get_trans_data(struct digital_dev *digital, uint8_t *buf, uint8_t len)
{
	if (!buf || len != DIGITAL_BYTES) {
		LOG_E("Invalid argument");
		return false;
	}

	pthread_mutex_lock(&digital->mutex);
	uint16_t current_value = digital->show_value;
	pthread_mutex_unlock(&digital->mutex);

	for (int j = 0; j < 4; j++) {
		uint8_t tmp = (current_value >> (12 - 4 * j)) & 0x0F;
		buf[2 * j] = 1 << j;
		buf[2 * j + 1] = get_map_value(tmp);
	}

	return true;
}

/**
 * @brief 初始化数码管
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_digital_init(void **p_priv)
{
	struct digital_dev *digital = malloc(sizeof(struct digital_dev));
	if (!digital) {
		LOG_E("Malloc digital failed");
		return false;
	}

	int ret = pthread_mutex_init(&digital->mutex, NULL);
	if (ret != 0) {
		LOG_E("Init mutex failed");
		goto err_free_digital;
	}

	digital->fd = open(DEV_PATH, O_WRONLY);
	if (digital->fd < 0) {
		LOG_E("Open file:%s failed", DEV_PATH);
		goto err_free_digital;
	}

	*p_priv = digital;
	digi_ins = digital;

	return true;

err_free_digital:
	free(digital);

	return false;
}

/**
 * @brief 去初始化数码管
 * 
 * @param priv 
 */
void app_digital_deinit(void *priv)
{
	struct digital_dev *digital = priv;

	if (digital->fd >= 0) {
		close(digital->fd);
		digital->fd = -1;
		LOG_I("Digital device closed.");
	}

	free(digital);
}

/**
 * @brief 测试代码 累加1s
 * 
 * @param digital 
 */
static void digital_test(struct digital_dev *digital)
{
	static size_t counter = 0;
	counter += APP_DIGITAL_TASK_PERIOD;
	if (counter >= 1000) {
		counter = 0;
		digital->show_value++;
	}
}

/**
 * @brief 数码管任务
 * 
 * @param priv 
 */
void app_digital_task(void *priv)
{
	struct digital_dev *digital = priv;

	if (digital->fd < 0)
		return;

	digital_test(digital);

	uint8_t buf[DIGITAL_BYTES] = { 0 };
	bool ret = get_trans_data(digital, buf, sizeof(buf));
	if (!ret)
		return;

	for (int i = 0; i < 4; i++) {
		ssize_t num = write(digital->fd, buf + i * 2, 2);
		if (num != 2) {
			LOG_E("Write data failed, num is %zd", num);
			close(digital->fd);
			digital->fd = -1;
			return;
		}
	}
}

/**
 * @brief 设置数码管显示值
 * 
 * @param value 十六进制值 如0x1234 就显示1234
 */
void app_change_digital(uint16_t value)
{
	if (!digi_ins)
		return;

	pthread_mutex_lock(&digi_ins->mutex);
	digi_ins->show_value = value;
	pthread_mutex_unlock(&digi_ins->mutex);
}
