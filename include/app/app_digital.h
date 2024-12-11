/**
 * @file app_digital.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 数码管
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

#ifndef _APP_DIGITAL_H
#define _APP_DIGITAL_H

#include <stdint.h>
#include <stdbool.h>

#define APP_DIGITAL_TASK_PERIOD (1U)

/**
 * @brief 初始化数码管
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_digital_init(void **p_priv);

/**
 * @brief 去初始化数码管
 * 
 * @param priv 私有数据指针
 */
void app_digital_deinit(void *priv);

/**
 * @brief 数码管任务
 * 
 * @param priv 私有数据指针
 */
void app_digital_task(void *priv);

/**
 * @brief 设置数码管显示值
 * 
 * @param value 十六进制值 如0x1234 就显示1234
 */
void app_change_digital(uint16_t value);

#endif /* _APP_DIGITAL_H */
