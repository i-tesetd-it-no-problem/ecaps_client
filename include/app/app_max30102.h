#ifndef _APP_MAX30102_H
#define _APP_MAX30102_H

#define APP_MAX30102_TASK_PERIOD (500U)

#include <stdbool.h>

/**
 * @brief 初始化采集模块
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_max30102_init(void **p_priv);

/**
 * @brief 采集任务
 * 
 * @param priv 私有数据指针
 */
void app_max30102_task(void *priv);

/**
 * @brief 去初始化采集模块
 * 
 * @param priv 私有数据指针
 */
void app_max30102_deinit(void *priv);

#endif