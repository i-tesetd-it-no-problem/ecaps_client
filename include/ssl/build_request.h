/**
 * @file build_request.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 构建HTTPS请求
 * @version 1.0
 * @date 2024-11-15
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

#ifndef __BUILD_REQUEST_H__
#define __BUILD_REQUEST_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * 构建一个HTTP GET请求字符串。
 *
 * @param url 完整的URL(例如 "https://www.example.com/test")
 * @param buffer 用于存储请求的缓冲区
 * @param buffer_size 缓冲区的大小
 * @return 成功返回0，失败返回非零值
 */
int build_get_request(const char* url, char* buffer, size_t buffer_size);

/**
 * 构建一个HTTP POST请求字符串。
 *
 * @param url 完整的URL(例如 "https://www.example.com/submit_sensor_data")
 * @param content_type 内容类型(例如 "application/json")
 * @param body 请求体内容(例如 "{\"name\":\"John\",\"age\":30}")，可以为NULL表示空请求体
 * @param buffer 用于存储请求的缓冲区
 * @param buffer_size 缓冲区的大小
 * @return 成功返回0，失败返回非零值
 */
int build_post_request(const char* url, const char* content_type, const char* body, char* buffer, size_t buffer_size);

/*********************************宏定义*********************************/

/********************************GET 请求********************************/
#define BUILD_GET(url, buffer, buffer_size) \
    build_get_request(url, buffer, buffer_size)   // GET请求

/********************************POST 请求********************************/
#define BUILD_POST_EMPTY(url, buffer, buffer_size) \
    build_post_request(url, "application/x-www-form-urlencoded", NULL, buffer, buffer_size) // 空POST请求

#define BUILD_POST_JSON(url, body, buffer, buffer_size) \
    build_post_request(url, "application/json", body, buffer, buffer_size) // JSON内容类型的POST请求

#endif /* __BUILD_REQUEST_H__ */
