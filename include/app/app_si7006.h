
#ifndef _APP_SI7006_H
#define _APP_SI7006_H

#include <stdint.h>
#include <stdbool.h>

#define APP_SI7006_TASK_PERIOD (2000U)

/**
 * @brief 初始化温湿度采集模块
 * 
 * @return bool
 */
bool app_si7006_init(void);

/**
 * @brief 关闭温湿度采集模块
 */
void app_si7006_deinit(void);

/**
 * @brief 采集温湿度数据
 * 
 */
void app_si7006_collect(void);

/**
 * @brief 获取湿度
 * 
 * @return float 湿度值 (%RH)
 */
float get_humidity(void);

/**
 * @brief 获取温度
 * 
 * @return float 温度值 (°C)
 */
float get_temperature(void);

#endif /* _APP_SI7006_H */
