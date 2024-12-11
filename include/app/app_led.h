/**
 * @file app_led.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief LED任务
 * @version 1.0
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

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
