#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include "app/app_si7006.h"
#include "utils/logger.h"

#include "json/sensor_json.h"

#define SI7006_FILE_PATH ("/dev/si7006")

enum si7006_cmd {
	MEASURE_RELATIVE_HUMIDITY = 0, // 测量湿度
	MEASURE_TEMEPERATURE,		   // 测量温度
	CMD_MAX,
};

struct si7006_task {
	int fd;			   // 文件描述符
	float humidity;	   // 转换后的湿度 (%RH)
	float temperature; // 转换后的温度 (°C)
};

static struct si7006_task si7006 = {
	.fd = -1,
	.humidity = 0.0f,
	.temperature = 0.0f,
};

/**
 * @brief 初始化温湿度采集模块
 * 
 * @return bool
 */
bool app_si7006_init(void)
{
	if (si7006.fd != -1) {
		LOG_W("SI7006 device already initialized.");
		return true;
	}

	si7006.fd = open(SI7006_FILE_PATH, O_RDWR);
	if (si7006.fd < 0) {
		LOG_E("Can't open device %s: %s", SI7006_FILE_PATH, strerror(errno));
		return false;
	}

	LOG_I("SI7006 device opened: %s", SI7006_FILE_PATH);
	return true;
}

/**
 * @brief 关闭温湿度采集模块
 */
void app_si7006_deinit(void)
{
	if (si7006.fd >= 0) {
		close(si7006.fd);
		si7006.fd = -1;
		LOG_I("SI7006 device closed.");
	}
}

/**
 * @brief 采集温湿度数据
 * 
 */
void app_si7006_collect(void)
{
	if (si7006.fd < 0) {
		LOG_E("SI7006 device not opened.");
		return;
	}

	uint8_t mea_cmd1 = MEASURE_RELATIVE_HUMIDITY;
	uint8_t mea_cmd2 = MEASURE_TEMEPERATURE;

	// 发送测量湿度指令
	ssize_t num = write(si7006.fd, &mea_cmd1, 1);
	if (num != 1) {
		LOG_E("Failed to write humidity measure command, err: %zd", num);
		return;
	}

	// 发送测量温度指令
	num = write(si7006.fd, &mea_cmd2, 1);
	if (num != 1) {
		LOG_E("Failed to write temperature measure command, err: %zd", num);
		return;
	}

	uint8_t read_data[4] = { 0 }; // 读取4字节数据
	num = read(si7006.fd, read_data, 4);
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

	si7006.humidity = actual_humidity;
	si7006.temperature = actual_temperature;

	LOG_I("Humidity: %.2f %%RH, Temperature: %.2f °C", si7006.humidity, si7006.temperature);

	get_sensor_data()->si7006.humidity = si7006.humidity;
	get_sensor_data()->si7006.temperature = si7006.temperature;
    
	return;
}

/**
 * @brief 获取湿度
 * 
 * @return float 湿度值 (%RH)
 */
float get_humidity(void)
{
	return si7006.humidity;
}

/**
 * @brief 获取温度
 * 
 * @return float 温度值 (°C)
 */
float get_temperature(void)
{
	return si7006.temperature;
}
