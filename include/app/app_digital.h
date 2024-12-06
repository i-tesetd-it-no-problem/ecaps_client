#ifndef _APP_DIGITAL_H
#define _APP_DIGITAL_H

#include <stdint.h>

/**
 * @brief pthread 线程形式
 * 
 * @param arg 参数
 * @return void* 返回NULL
 */
void *app_digital_task(void *arg);

/**
 * @brief 修改显示值 原样显示16进制形式 例如输入0x1234 就显示1234
 * 
 * @param value 
 */
void app_change_digital(uint16_t value);

/**
 * @brief 停止线程
 * 
 */
void app_digital_stop(void);

#endif /* _APP_DIGITAL_H */