
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
 * @brief 初始化LED模块
 * 
 * @return bool
 */
bool app_led_init(void);

/**
 * @brief 关闭LED模块
 */
void app_led_deinit(void);

/**
 * @brief 处理LED任务
 * 
 * 此函数应在主事件循环中由对应的定时器触发时调用。
 */
void app_led_process(void);
/**
 * @brief 切换LED状态
 * 
 * @param state 目标LED状态
 */
void led_chg_state(enum led_state state);

#endif /* _APP_LED_H */
