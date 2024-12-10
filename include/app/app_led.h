
#ifndef _APP_LED_H
#define _APP_LED_H

#include <stdint.h>
#include <stdbool.h>

#define APP_LED_TASK_PERIOD_MS (5U)

enum led_state {
	LED_STATE_NORMAL, // 正常状态
	LED_STATE_SCROLL, // 跑马灯状态

	LED_STATE_MAX, // 无状态
};

/**
 * @brief 初始化LED
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_led_init(void **p_priv);

/**
 * @brief 去初始化LED
 * 
 * @param priv 私有数据指针
 */
void app_led_deinit(void *priv);

/**
 * @brief LED任务
 * 
 * @param priv 私有数据指针
 */
void app_led_task(void *priv);

/**
 * @brief 切换LED状态
 * 
 * @param state 目标LED状态
 */
void led_chg_state(enum led_state state);

#endif /* _APP_LED_H */
