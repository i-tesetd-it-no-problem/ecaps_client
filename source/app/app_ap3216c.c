#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include "app/app_ap3216c.h"
#include "utils/logger.h"
#include "json/sensor_json.h"

#define AP3216C_FILE_PATH ("/dev/ap3216c")

struct ap3216c_task {
	int fd;		  // 文件描述符
	uint16_t ir;  // 红外线传感器(IR)
	uint16_t als; // 环境光传感器(ALS)
	uint16_t ps;  // 接近传感器(PS)
};

static struct ap3216c_task ap3216c = {
	.fd = -1,
	.ir = 0,
	.als = 0,
	.ps = 0,
};

/**
 * @brief 初始化温湿度采集模块
 * 
 * @return bool
 */
bool app_ap3216c_init(void)
{
	if (ap3216c.fd != -1) {
		LOG_W("ap3216c device already initialized.");
		return true;
	}

	ap3216c.fd = open(AP3216C_FILE_PATH, O_RDWR);
	if (ap3216c.fd < 0) {
		LOG_E("Can't open device %s", AP3216C_FILE_PATH);
		return false;
	}

	LOG_I("ap3216c device opened: %s", AP3216C_FILE_PATH);
	return true;
}

/**
 * @brief 关闭温湿度采集模块
 */
void app_ap3216c_deinit(void)
{
	if (ap3216c.fd >= 0) {
		close(ap3216c.fd);
		ap3216c.fd = -1;
		LOG_I("ap3216c device closed.");
	}
}

/**
 * @brief 采集温湿度数据
 * 
 */
void app_ap3216c_collect(void)
{
	if (ap3216c.fd < 0) {
		LOG_E("ap3216c device not opened.");
		return;
	}

	uint8_t buf[6] = { 0 };
	int num = read(ap3216c.fd, buf, sizeof(buf));
	if (num != sizeof(buf)) {
		LOG_E("Failed to read data, err: %zd", num);
		return;
	}

	if (buf[0] & 0X80) /* IR_OF 位为 1,则数据无效 */
		ap3216c.ir = 0.0;
	else
		ap3216c.ir = ((unsigned short)buf[1] << 2) | (buf[0] & 0X03);

	ap3216c.als = ((unsigned short)buf[3] << 8) | buf[2];

	if (buf[4] & 0x40) /* IR_OF 位为 1,则数据无效 */
		ap3216c.ps = 0;
	else
		ap3216c.ps = ((unsigned short)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);

	LOG_I("ir:%d als:%d ps:%d", ap3216c.ir, ap3216c.als, ap3216c.ps);

	get_sensor_data()->ap3216c.illuminance = ap3216c.als;
	get_sensor_data()->ap3216c.infrared = ap3216c.ir;
	get_sensor_data()->ap3216c.proximity = ap3216c.ps;

	return;
}

/**
 * @brief 红外线传感器值
 * 
 * @return
 */
uint16_t get_ir(void)
{
	return ap3216c.ir;
}

/**
 * @brief 环境光传感器值
 * 
 * @return
 */
uint16_t get_ials(void)
{
	return ap3216c.als;
}

/**
 * @brief 接近传感器值
 * 
 * @return
 */
uint16_t get_ps(void)
{
	return ap3216c.ps;
}