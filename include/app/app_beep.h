/**
 * @file app_beep.h
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

#ifndef _APP_BEEP_
#define _APP_BEEP_

#include <stdbool.h>

/**
 * @brief 初始化蜂鸣器
 * 
 * @return true 
 * @return false 
 */
bool app_beep_init(void);

/**
 * @brief 开启/关闭蜂鸣器
 * 
 * @param enable 
 */
void beep_control(bool enable);

/**
 * @brief 提升音量（增加响度）
 * 
 */
void app_beep_higher(void);

/**
 * @brief 降低音量（减小响度）
 * 
 */
void app_beep_lower(void);

/**
 * @brief 提升音调（增加频率）
 * 
 */
void app_beep_higher_tone(void);

/**
 * @brief 降低音调（降低频率）
 * 
 */
void app_beep_lower_tone(void);

#endif /* _APP_BEEP_ */