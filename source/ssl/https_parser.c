/**
 * @file https_parser.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ssl/https_parser.h"
#include "utilities/logger.h"
#include <ctype.h>

#define HTTPS_MAX_HEADER_SIZE   (512)  // 头部最大长度

// 解析状态
enum parser_state {
    PARSER_STATE_INIT,          // 初始状态
    PARSER_STATE_HEADER,        // 解析头部
    PARSER_STATE_BODY,          // 接收主体
    PARSER_STATE_COMPLETE,      // 解析完成
    PARSER_STATE_ERROR          // 发生错误
};

struct _resp_parser {
    enum parser_state state;                    // 当前解析器状态
    size_t content_length;                      // Content-Length值
    size_t header_length;                       // 头部长度
    size_t body_received;                       // 已接收的主体长度
    size_t total_received;                      // 总共接收的字节数
    char header_buffer[HTTPS_MAX_HEADER_SIZE];  // 静态缓冲区存储头部
};

/**
 * @brief 不区分大小写的查找子字符串
 * 
 * @param haystack 主字符串
 * @param needle 子字符串
 * @return char* 如果找到返回子字符串的指针，否则返回NULL
 */
char *strcasestr(const char *haystack, const char *needle)
{
    if (!*needle) {
        return (char *)haystack; // 如果needle为空，直接返回haystack
    }

    while (*haystack) {
        if (tolower(*haystack) == tolower(*needle)) {
            const char *h = haystack, *n = needle;
            while (*n && tolower(*h) == tolower(*n)) {
                h++;
                n++;
            }
            if (!*n) {  // 如果完全匹配
                return (char *)haystack;
            }
        }
        haystack++;
    }
    return NULL;
}

/**
 * @brief 初始化HTTPS解析器
 * 
 * @return 成功返回0，失败返回-1
 */
int https_response_parser_init(https_resp_handle *parser)
{
    if(!parser)
        return -1;

    *parser = malloc(sizeof(struct _resp_parser));
    if (!parser) {
        LOG_E("Failed to allocate memory for HTTPS response parser.");
        return -1;
    }
    
    (*parser)->state = PARSER_STATE_INIT;
    (*parser)->content_length = 0;
    (*parser)->header_length = 0;
    (*parser)->body_received = 0;
    (*parser)->total_received = 0;
    memset((*parser)->header_buffer, 0, HTTPS_MAX_HEADER_SIZE);
    
    LOG_D("HTTPS response parser created successfully.");
    return 0;
}

/**
 * @brief 释放HTTP解析器
 * 
 * @param parser 解析器指针
 */
void https_response_parser_free(https_resp_handle *parser)
{
    if (parser) {
        LOG_D("Freeing HTTPS response parser.");
        free(*parser);
    }
}

/**
 * @brief 更新HTTP解析器状态
 * 
 * @param parser 解析器
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return int 0 成功，-1 失败
 */
int https_response_parser_update(https_resp_handle parser, const unsigned char *data, size_t len)
{
    if (!parser || !data) {
        LOG_E("null pointer\n");
        return -1;
    }

    LOG_D("Updating parser with %zu bytes of data.", len);
    size_t i = 0;
    parser->total_received += len;

    while (i < len) {
        switch (parser->state) {
            case PARSER_STATE_INIT:
                LOG_D("Transitioning state from INIT to HEADER.");
                parser->state = PARSER_STATE_HEADER;
                parser->header_length = 0;
                break;

            case PARSER_STATE_HEADER:
                // 将数据复制到头部缓冲区
                while (i < len) {
                    // 检查缓冲区是否有足够空间
                    if (parser->header_length >= HTTPS_MAX_HEADER_SIZE - 1) { // 留出1字节给'\0'
                        LOG_E("Header buffer overflow. Header length: %zu bytes.", parser->header_length);
                        parser->state = PARSER_STATE_ERROR;
                        return -1;
                    }
                    parser->header_buffer[parser->header_length++] = data[i];
                    
                    // 检查头部结束标志
                    if (parser->header_length >= 4 &&
                        parser->header_buffer[parser->header_length - 4] == '\r' &&
                        parser->header_buffer[parser->header_length - 3] == '\n' &&
                        parser->header_buffer[parser->header_length - 2] == '\r' &&
                        parser->header_buffer[parser->header_length - 1] == '\n') {
                        
                        // 确保以NULL结尾
                        if (parser->header_length >= HTTPS_MAX_HEADER_SIZE) {
                            LOG_E("Header buffer full before null-terminating.");
                            parser->state = PARSER_STATE_ERROR;
                            return -1;
                        }
                        parser->header_buffer[parser->header_length] = '\0';

                        parser->state = PARSER_STATE_BODY;
                        LOG_D("Transitioning state from HEADER to BODY.");

                        // 解析Content-Length
                        char *content_length_str = strcasestr(parser->header_buffer, "Content-Length:");
                        if (content_length_str) {
                            content_length_str += strlen("Content-Length:");
                            // 跳过可能的空白字符
                            while (*content_length_str == ' ' || *content_length_str == '\t') {
                                content_length_str++;
                            }
                            char *endptr;
                            long content_length = strtol(content_length_str, &endptr, 10);
                            if (endptr == content_length_str || content_length < 0 || content_length > HTTPS_MAX_CONTENT_SIZE) {
                                LOG_E("Invalid Content-Length value: '%s'.", content_length_str);
                                parser->state = PARSER_STATE_ERROR;
                                return -1;
                            }
                            parser->content_length = (size_t)content_length;
                            LOG_D("Parsed Content-Length: %zu bytes.", parser->content_length);
                        } else {
                            LOG_E("Content-Length header not found.");
                            // 未找到Content-Length，可能是分块传输，暂不支持
                            parser->state = PARSER_STATE_ERROR;
                            return -1;
                        }


                        // 计算已读取的主体数据长度
                        size_t remaining = len - i - 1; // -1 因为i已经指向了当前字符
                        LOG_D("Remaining bytes after header: %zu.", remaining);

                        // 更新已接收的主体数据长度
                        if (remaining > 0) {
                            if (parser->body_received + remaining < parser->body_received) {
                                // 整数溢出
                                LOG_E("Integer overflow detected while updating body_received.");
                                parser->state = PARSER_STATE_ERROR;
                                return -1;
                            }
                            parser->body_received += remaining;
                            LOG_D("Updated body_received: %zu bytes.", parser->body_received);
                        }

                        i = len; // 跳出循环，因为剩余数据已处理
                        break;
                    }
                    i++;
                }
                break;

            case PARSER_STATE_BODY:
                // 更新已接收的主体数据长度
                {
                    size_t body_chunk = len - i;
                    LOG_D("Processing body chunk of %zu bytes.", body_chunk);
                    if (parser->body_received + body_chunk < parser->body_received) {
                        // 整数溢出
                        LOG_E("Integer overflow detected while updating body_received.");
                        parser->state = PARSER_STATE_ERROR;
                        return -1;
                    }
                    parser->body_received += body_chunk;
                    i = len; // 消耗完所有数据

                    if (parser->body_received >= parser->content_length) {
                        parser->state = PARSER_STATE_COMPLETE;
                        LOG_D("Parsing complete. Total body received: %zu bytes.", parser->body_received);
                    }
                }
                break;

            case PARSER_STATE_COMPLETE:
                LOG_D("Parser state is COMPLETE. No further processing.");
                return 0;

            case PARSER_STATE_ERROR:
                LOG_E("Parser is in ERROR state. Aborting update.");
                return -1;

            default:
                LOG_E("Unknown parser state: %d. Transitioning to ERROR state.", parser->state);
                parser->state = PARSER_STATE_ERROR;
                return -1;
        }
    }
    return 0;
}

/**
 * @brief 检查HTTP解析器是否完成
 * 
 * @param parser 解析器
 * @return int -1 错误，0 未完成，1 完成
 */
int https_response_parser_is_complete(const https_resp_handle parser)
{
    if (!parser) {
        LOG_E("Parser is NULL.");
        return -1;
    }

    if (parser->state == PARSER_STATE_BODY && parser->body_received >= parser->content_length) {
        LOG_D("Parser is complete. Body received: %zu bytes.", parser->body_received);
        return 1;
    }
    
    if (parser->state == PARSER_STATE_COMPLETE) {
        LOG_D("Parser state is COMPLETE.");
        return 1;
    }
    
    if (parser->state == PARSER_STATE_ERROR) {
        LOG_W("Parser state is ERROR.");
        return -1;
    }
    
    LOG_D("Parser is not complete. Current state: %d.", parser->state);
    return 0;
}

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
                                   const unsigned char **body_ptr, size_t *body_len)
{
    if (!parser || !buffer || !body_ptr || !body_len) {
        LOG_E("Invalid input parameters.");
        return -1;
    }

    if (parser->state != PARSER_STATE_COMPLETE && parser->body_received < parser->content_length) {
        LOG_W("Parser not complete.");
        return -1;
    }

    // 查找头部结束的位置
    const char *header_end = strstr((const char *)buffer, "\r\n\r\n");
    if (!header_end) {
        LOG_E("Header end delimiter '\\r\\n\\r\\n' not found.");
        return -1;
    }

    size_t header_size = header_end - (const char *)buffer + 4; // 包括 "\r\n\r\n"
    LOG_D("Header size detected: %zu bytes.", header_size);

    // 确保缓冲区长度足够
    if (buffer_len < header_size + parser->content_length) {
        LOG_E("Buffer size is insufficient.");
        return -1;
    }

    // 跳过头部，返回正文部分
    *body_ptr = (const unsigned char *)(buffer + header_size);
    *body_len = parser->content_length;

    LOG_D("Extracted body of %zu bytes.", *body_len);
    return 0;
}