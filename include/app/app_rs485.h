#ifndef _APP_RS485_H
#define _APP_RS485_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define APP_RS485_TASK_PERIOD (5U)

/**
 * @brief RS485初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_rs485_init(void **p_priv);

/**
 * @brief 去初始化rs485
 * 
 * @param priv 私有数据指针
 */
void app_rs485_deinit(void *priv);

/**
 * @brief rs485任务处理
 * 
 * @param priv 私有数据指针
 */
void app_rs485_task(void *priv);

/**
 * @brief 发送数据
 * 
 * @param buf 数据缓冲
 * @param len 数据长度
 */
void app_rs485_write(uint8_t *buf, size_t len);

#endif