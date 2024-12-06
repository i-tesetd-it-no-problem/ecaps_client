/**
 * @file app_si7006.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 温湿度采集
 * @version 1.0
 * @date 2024-12-06
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

#ifndef _APP_SI7006_H
#define _APP_SI7006_H

#include <stdint.h>

/**
 * @brief 采集温湿度任务
 * 
 * @param arg 线程参数
 * @return void* 返回 NULL
 */
void *app_si7006_task(void *arg);

/**
 * @brief 停止采集任务
 * 
 */
void app_si7006_task_stop(void);

/**
 * @brief 获取湿度
 * 
 * @return float 
 */
float get_humidity(void);

/**
 * @brief 获取温度
 * 
 * @return float 
 */
float get_temperature(void);

#endif /* _APP_SI7006_H */