/**
 * @file app_ap3216c.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 光照强度/红外强度/接近距离 采集模块
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

#ifndef _APP_AP3216C_H
#define _APP_AP3216C_H

#include <stdbool.h>
#include <stdint.h>

#define APP_AP3216C_TASK_PEIOD (100U)

/**
 * @brief 初始化采集模块
 * 
 * @param p_priv 二级私有数据指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_ap3216c_init(void **p_priv);

/**
 * @brief 去初始化采集模块
 * 
 * @param priv 私有数据指针
 */
void app_ap3216c_deinit(void *priv);

/**
 * @brief 采集任务
 * 
 * @param priv 
 */
void app_ap3216c_task(void *priv);

#endif /* _APP_AP3216C_H */