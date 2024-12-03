#ifndef _APP_LED_H
#define _APP_LED_H

enum led_state {
	LED_STATE_NORMAL, // 正常状态
	LED_STATE_SCROL,  // 跑马灯

	LED_STATE_NONE,  // 无
};

/**
 * @brief LED 任务线程 pthread函数指针形式
 * 
 * @param arg 线程参数
 * @return void* 返回 NULL
 */
void *app_led_task(void *arg);

/**
 * @brief 停止LED任务
 * 
 */
void stop_led_task(void);

/**
 * @brief 切换LED状态
 * 
 * @param state 
 */
void led_chg_state(enum led_state state);

#endif /* _APP_LED_H */