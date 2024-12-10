#ifndef _APP_AP3216C_H
#define _APP_AP3216C_H

#include <stdbool.h>
#include <stdint.h>

#define APP_AP3216C_TASK_PEIOD (100U)

/**
 * @brief 初始化采集模块
 * 
 * @param p_priv 二级私有数据指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_ap3216c_init(void **p_priv);

/**
 * @brief 去初始化采集模块
 * 
 * @param priv 私有数据指针
 */
void app_ap3216c_deinit(void *priv);

/**
 * @brief 采集任务
 * 
 * @param priv 
 */
void app_ap3216c_task(void *priv);

#endif /* _APP_AP3216C_H */