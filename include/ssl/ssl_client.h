/**
 * @file ssl_client.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief SSL客户端接口
 * @version 1.0
 * @date 2024-11-13
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

#ifndef __SSL_CLIENT_H__
#define __SSL_CLIENT_H__

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct ssl_client_t *ssl_handle; // 客户端句柄

/**
 * @brief 初始化SSL客户端
 * 
 * @param client 指向ssl_handle的指针
 * @return int 成功返回0,失败返回-1
 */
int ssl_client_init(ssl_handle *client);

/**
 * @brief 释放SSL客户端
 * 
 * @param client 
 */
void ssl_client_free(ssl_handle client);

/**
 * @brief 与服务器建立连接
 * 
 * @param client 客户端句柄
 * @param url 服务器URL
 * @return int 成功返回0,失败返回错误码
 */
int ssl_client_connect(ssl_handle client, const char *url);

/**
 * @brief 关闭SSL连接
 * 
 * @param client SSL客户端句柄
 */
void ssl_client_close(ssl_handle client);

/**
 * @brief Get 请求
 * 
 * @param client SSL客户端
 * @param url 请求的URL
 * @param response_body 调用者提供的缓冲区,用于存储响应主体
 * @param response_buffer_size 响应缓冲区的大小
 * @param response_body_len 实际接收到的响应主体长度
 * @return int 成功返回0,失败返回错误码
 */
int ssl_client_get(ssl_handle client, const char *url, unsigned char *response_body, size_t response_buffer_size, size_t *response_body_len);

/**
 * @brief Post 请求
 * 
 * @param client SSL客户端
 * @param url 请求的URL
 * @param json_body JSON格式的请求主体
 * @param response_body 调用者提供的缓冲区,用于存储响应主体
 * @param response_buffer_size 响应缓冲区的大小
 * @param response_body_len 实际接收到的响应主体长度
 * @return int 成功返回0,失败返回错误码
 */
int ssl_client_post(ssl_handle client, const char *url, const char *json_body,
                    unsigned char *response_body, size_t response_buffer_size, size_t *response_body_len);


// 一次通信的完整流程
// 1. 初始化SSL客户端
// 2. 与服务器建立连接
// 3. 发送请求
// 4. 自行处理响应
// 5. 关闭SSL连接
// 6. 释放SSL客户端

// 自行决定是否长时间保持连接,或是每次请求后关闭连接

#endif /* __SSL_CLIENT_H__ */
