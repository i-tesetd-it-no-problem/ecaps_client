#ifndef _APP_DIGITAL_H
#define _APP_DIGITAL_H

#include <stdint.h>
#include <stdbool.h>

#define APP_DIGITAL_TASK_PERIOD (1U)

bool app_digital_init(void);

void app_digital_deinit(void);

void app_digital_task(void);

/**
 * @brief 设置数码管
 * 
 * @param value 
 */
void app_change_digital(uint16_t value);

/**
 * @brief 获取数码管的值
 * 
 * @return uint16_t 
 */
uint16_t app_get_digital_value(void);

#endif /* _APP_DIGITAL_H */
