/**
 * @file app_pwm.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief PWM通用接口
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

#ifndef _APP_PWM_H
#define _APP_PWM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief 导出PWM
 * 
 * @param pwm_chip PWM编号
 * @param pwm_num PWM通道号
 * @return bool
 */
bool export_pwm(uint8_t pwm_chip, uint8_t pwm_num);

/**
 * @brief 设置PWM周期
 * 
 * @param pwm_chip PWM编号
 * @param pwm_num PWM通道号
 * @param period_ns 纳秒
 * @return bool 
 */
bool set_pwm_period(uint8_t pwm_chip, uint8_t pwm_num, size_t period_ns);

/**
 * @brief 设置PWM占空值
 * 
 * @param pwm_chip PWM编号
 * @param pwm_num PWM通道号
 * @param duty_ns 纳秒
 * @return bool 
 */
bool set_pwm_duty_cycle(uint8_t pwm_chip, uint8_t pwm_num, size_t duty_ns);

/**
 * @brief 设置PWM占空值
 * 
 * @param pwm_chip PWM编号
 * @param pwm_num PWM通道号
 * @param enable 开启/关闭
 * @return bool 
 */
bool enable_pwm(uint8_t pwm_chip, uint8_t pwm_num, bool enable);

#endif /* _APP_PWM_H */