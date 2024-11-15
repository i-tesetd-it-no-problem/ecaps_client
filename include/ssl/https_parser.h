/**
 * @file https_parser.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief HTTPS响应解析
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

#ifndef HTTPS_PARSER_H
#define HTTPS_PARSER_H

#include <stddef.h>

typedef struct _resp_parser *https_resp_handle; // HTTPS解析句柄

/**
 * @brief 初始化HTTPS解析器
 * 
 * @return 成功返回0，失败返回-1
 */
int https_response_parser_init(https_resp_handle *parser);

/**
 * @brief 释放HTTP解析器
 * 
 * @param parser 解析器指针
 */
void https_response_parser_free(https_resp_handle *parser);

/**
 * @brief 更新HTTP解析器状态
 * 
 * @param parser 解析器
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return int 0 成功，-1 失败
 */
int https_response_parser_update(https_resp_handle parser, const unsigned char *data, size_t len);

/**
 * @brief 检查HTTP解析器是否完成
 * 
 * @param parser 解析器
 * @return int -1 错误，0 未完成，1 完成
 */
int https_response_parser_is_complete(const https_resp_handle parser);

/**
 * @brief 获取HTTP响应主体数据
 * 
 * @param parser 解析器
 * @param buffer 响应数据缓冲区
 * @param buffer_len 缓冲区长度
 * @param body_ptr 输出：主体数据指针
 * @param body_len 输出：主体数据长度
 * @return int -1 错误，0 成功
 */
int https_response_parser_get_body(const https_resp_handle parser, const unsigned char *buffer, size_t buffer_len,
                                   const unsigned char **body_ptr, size_t *body_len);

#endif // HTTPS_PARSER_H
