#ifndef _APP_DIGITAL_H
#define _APP_DIGITAL_H

#include <stdint.h>
#include <stdbool.h>

#define APP_DIGITAL_TASK_PERIOD (1U)

/**
 * @brief 初始化数码管
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_digital_init(void **p_priv);

/**
 * @brief 去初始化数码管
 * 
 * @param priv 私有数据指针
 */
void app_digital_deinit(void *priv);

/**
 * @brief 数码管任务
 * 
 * @param priv 私有数据指针
 */
void app_digital_task(void *priv);

/**
 * @brief 设置数码管显示值
 * 
 * @param value 十六进制值 如0x1234 就显示1234
 */
void app_change_digital(uint16_t value);

#endif /* _APP_DIGITAL_H */
