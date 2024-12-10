
#ifndef _APP_SI7006_H
#define _APP_SI7006_H

#include <stdint.h>
#include <stdbool.h>

#define APP_SI7006_TASK_PERIOD (2000U)

/**
 * @brief 初始化采集模块
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_si7006_init(void **p_priv);

/**
 * @brief 去初始化采集模块
 * 
 * @param priv 私有数据指针
 */
void app_si7006_deinit(void *priv);

/**
 * @brief 采集任务
 * 
 * @param priv 私有数据指针
 */
void app_si7006_task(void *priv);

#endif /* _APP_SI7006_H */
