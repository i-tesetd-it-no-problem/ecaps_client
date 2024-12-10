/**
 * @file app_motor.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 峰鸣器
 * @version 1.0
 * @date 2024-12-03
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

#ifndef _APP_MOTOR_
#define _APP_MOTOR_

#include "app/app_pwm.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_MOTOR_PERIOD (100U)			// 最大频率
#define MAX_MOTOR_DUTY MAX_MOTOR_PERIOD // 最大占空值

/**
 * @brief 初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_motor_init(void **p_priv);

/**
 * @brief 开启/关闭
 * 
 * @param enable 
 */
void motor_control(bool enable);

/**
 * @brief 提高占空比
 * 
 */
void app_motor_add_duty(size_t duty);

/**
 * @brief 减少占空比
 * 
 */
void app_motor_sub_duty(size_t duty);

/**
 * @brief 提高周期
 * 
 */
void app_motor_add_period(size_t period);

/**
 * @brief 减少周期
 * 
 */
void app_motor_sub_period(size_t period);

#endif /* _APP_MOTOR_ */