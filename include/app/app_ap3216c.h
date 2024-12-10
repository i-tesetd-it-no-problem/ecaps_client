#ifndef _APP_AP3216C_H
#define _APP_AP3216C_H

#include <stdbool.h>
#include <stdint.h>

#define APP_AP3216C_TASK_PEIOD (100U)

/**
 * @brief 初始化温湿度采集模块
 * 
 * @return bool
 */
bool app_ap3216c_init(void);

/**
 * @brief 关闭温湿度采集模块
 */
void app_ap3216c_deinit(void);

/**
 * @brief 采集温湿度数据
 * 
 */
void app_ap3216c_collect(void);

/**
 * @brief 红外线传感器值
 * 
 * @return
 */
uint16_t get_ir(void);

/**
 * @brief 环境光传感器值
 * 
 * @return
 */
uint16_t get_ials(void);

/**
 * @brief 接近传感器值
 * 
 * @return
 */
uint16_t get_ps(void);

#endif /* _APP_AP3216C_H */