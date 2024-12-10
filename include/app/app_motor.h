/**
 * @file app_motor.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 振动马达
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

#include <stdbool.h>

/**
 * @brief 初始化震动马达
 * 
 * @return true 
 * @return false 
 */
bool app_motor_init(void);

/**
 * @brief 开启/关闭震动马达
 * 
 * @param enable 
 */
void vibration_motor_control(bool enable);

/**
 * @brief 提升震动强度(增加占空比)
 * 
 */
void app_motor_stronger(void);

/**
 * @brief 降低震动强度(减少占空比)
 * 
 */
void app_motor_weaker(void);

/**
 * @brief 提升震动频率(增加振动速度)
 * 
 */
void app_motor_faster(void);

/**
 * @brief 降低震动频率(减慢振动速度)
 * 
 */
void app_motor_slower(void);

#endif /* _APP_MOTOR_ */
