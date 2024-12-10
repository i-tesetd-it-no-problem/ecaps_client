#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "app/app_ap3216c.h"
#include "utils/logger.h"
#include "json/sensor_json.h"

#define AP3216C_FILE_PATH ("/dev/ap3216c")

struct ap3216c_task {
	int fd; // 文件描述符
};

/**
 * @brief 初始化采集模块
 * 
 * @param p_priv 二级私有数据指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_ap3216c_init(void **p_priv)
{
	if (!p_priv)
		return false;

	struct ap3216c_task *ap3216c = malloc(sizeof(struct ap3216c_task));
	if (!ap3216c) {
		LOG_E("Malloc ap3216c failed");
		return false;
	}

	ap3216c->fd = open(AP3216C_FILE_PATH, O_RDWR);
	if (ap3216c->fd < 0) {
		LOG_E("Can't open device %s", AP3216C_FILE_PATH);
		goto err_free_ap3216c;
	}

	*p_priv = ap3216c;
	return true;

err_free_ap3216c:
	free(ap3216c);

	return false;
}

/**
 * @brief 去初始化采集模块
 * 
 * @param priv 私有数据指针
 */
void app_ap3216c_deinit(void *priv)
{
	struct ap3216c_task *ap3216c = priv;

	if (ap3216c->fd >= 0) {
		close(ap3216c->fd);
		ap3216c->fd = -1;
	}

	free(ap3216c);
}

/**
 * @brief 采集任务
 * 
 * @param priv 
 */
void app_ap3216c_task(void *priv)
{
	struct ap3216c_task *ap3216c = priv;

	if (ap3216c->fd < 0) {
		LOG_E("ap3216c device not opened.");
		return;
	}

	uint8_t buf[6] = { 0 };
	int num = read(ap3216c->fd, buf, sizeof(buf));
	if (num != sizeof(buf)) {
		LOG_E("Failed to read data, err: %zd", num);
		return;
	}

	uint16_t ir;  // 红外线传感器(IR)
	uint16_t als; // 环境光传感器(ALS)
	uint16_t ps;  // 接近传感器(PS)

	if (buf[0] & 0X80) /* IR_OF 位为 1,则数据无效 */
		ir = 0.0;
	else
		ir = ((unsigned short)buf[1] << 2) | (buf[0] & 0X03);

	als = ((unsigned short)buf[3] << 8) | buf[2];

	if (buf[4] & 0x40) /* IR_OF 位为 1,则数据无效 */
		ps = 0;
	else
		ps = ((unsigned short)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);

	LOG_I("ir:%d als:%d ps:%d", ir, als, ps);

	get_sensor_data()->ap3216c.illuminance = als;
	get_sensor_data()->ap3216c.infrared = ir;
	get_sensor_data()->ap3216c.proximity = ps;

	return;
}
