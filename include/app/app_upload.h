#ifndef _APP_UPLOAD_H
#define _APP_UPLOAD_H

#include <stdbool.h>

// #define APP_UPLOAD_TASK_PERIOD (60 * 1000) // 1分钟
#define APP_UPLOAD_TASK_PERIOD (1000)

/**
 * @brief SSL客户端初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_upload_init(void **p_priv);

/**
 * @brief SSL客户端去初始化
 * 
 * @param priv 私有数据指针
 */
void app_upload_deinit(void *priv);

/**
 * @brief SSL客户端任务
 * 
 * @param priv 私有数据指针
 */
void app_upload_task(void *priv);

#endif /* _APP_UPLOAD_H */