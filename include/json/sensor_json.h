/**
 * @file sensor_json.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 所有传感器数据
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

#ifndef __CREATE_JSON_H__
#define __CREATE_JSON_H__

#include "json/sensor.h"

/**
 * @brief 获取传感器数据结构体指针(单例模式)
 * 
 * @return struct sensor_data* 
 */
struct sensor_data *get_sensor_data(void);

/**
 * @brief 获取当前传感器数据转换的Json字符串 (遵循 Docs/docs_third_part/cJson/JsonSchema.json 规范)
 * 
 * @return char* 成功返回Json字符串,失败返回NULL
 */
char* get_cur_sensor_json(void);

#endif /* __CREATE_JSON_H__ */