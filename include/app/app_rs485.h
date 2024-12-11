/**
 * @file app_rs485.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief RS485串口任务
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

#ifndef _APP_RS485_H
#define _APP_RS485_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define APP_RS485_TASK_PERIOD (5U)

/**
 * @brief RS485初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_rs485_init(void **p_priv);

/**
 * @brief 去初始化rs485
 * 
 * @param priv 私有数据指针
 */
void app_rs485_deinit(void *priv);

/**
 * @brief rs485任务处理
 * 
 * @param priv 私有数据指针
 */
void app_rs485_task(void *priv);

/**
 * @brief 发送数据
 * 
 * @param buf 数据缓冲
 * @param len 数据长度
 */
void app_rs485_write(uint8_t *buf, size_t len);

#endif