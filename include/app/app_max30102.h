#ifndef _APP_MAX30102_H
#define _APP_MAX30102_H

#define APP_MAX30102_TASK_PERIOD (500U)

#include <stdbool.h>

bool app_max30102_init(void);
void app_max30102_task(void);
void app_max30102_deinit(void);

int get_SpO2(void);		  // 获取血氧
int get_heart_rate(void); // 获取心率

#endif