/**
 * @file build_request.c
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

#include "ssl/build_request.h"
#include "ssl/url_parser.h"
#include "utils/logger.h"

/**
 * @brief 构建HTTPS GET请求字符串。
 * 
 * @param url 
 * @param buffer 
 * @param buffer_size 
 * @return int 
 */
int build_get_request(const char* url, char* buffer, size_t buffer_size)
{
    if (!url) {
        LOG_E("URL cannot be NULL\n");
        return -1;
    }

    // 解析URL
    url_info_t *url_info = parse_url(url);
    if (!url_info) {
        LOG_E("Failed to parse URL\n");
        return -1;
    }

    // 基本请求行
    const char* request_line_template = "GET %s HTTP/1.1\r\n";

    // Host头
    char host_header[256];
    if ((strcmp(url_info->protocol, "http") == 0 && strcmp(url_info->port, "80") != 0) ||
        (strcmp(url_info->protocol, "https") == 0 && strcmp(url_info->port, "443") != 0)) {
        // 非默认端口，包含端口号
        snprintf(host_header, sizeof(host_header), "Host: %s:%s\r\n", url_info->host, url_info->port);
    } else {
        // 默认端口，不包含端口号
        snprintf(host_header, sizeof(host_header), "Host: %s\r\n", url_info->host);
    }

    // 连接保持
    const char* connection_header = "Connection: keep-alive\r\n";
    // 结束标志
    const char* end_of_headers = "\r\n";

    // 构建请求
    size_t offset = 0;
    int n = snprintf(buffer + offset, buffer_size - offset, request_line_template, url_info->route);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at request line\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    n = snprintf(buffer + offset, buffer_size - offset, "%s", host_header);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at host header\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    n = snprintf(buffer + offset, buffer_size - offset, "%s", connection_header);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at connection header\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    n = snprintf(buffer + offset, buffer_size - offset, "%s", end_of_headers);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at end of headers\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    // 确保字符串以空字符结尾
    if (offset < buffer_size) {
        buffer[offset] = '\0';
    } else {
        LOG_E("Buffer overflow detected at null-termination\n");
        free_url_info(url_info);
        return -1;
    }

    free_url_info(url_info);
    return 0;
}

/**
 * 构建一个HTTP POST请求字符串。
 */
int build_post_request(const char* url, const char* content_type, const char* body, char* buffer, size_t buffer_size)
{
    if (!url) {
        LOG_E("URL cannot be NULL\n");
        return -1;
    }

    // 解析URL
    url_info_t *url_info = parse_url(url);
    if (!url_info) {
        LOG_E("Failed to parse URL\n");
        return -1;
    }

    // 基本请求行
    const char* request_line_template = "POST %s HTTP/1.1\r\n";

    // Host头
    char host_header[256];
    if ((strcmp(url_info->protocol, "http") == 0 && strcmp(url_info->port, "80") != 0) ||
        (strcmp(url_info->protocol, "https") == 0 && strcmp(url_info->port, "443") != 0)) {
        snprintf(host_header, sizeof(host_header), "Host: %s:%s\r\n", url_info->host, url_info->port);
    } else {
        snprintf(host_header, sizeof(host_header), "Host: %s\r\n", url_info->host);
    }

    // 内容类型，允许content_type为空
    char content_type_header[256] = "";
    if (content_type) {
        snprintf(content_type_header, sizeof(content_type_header), "Content-Type: %s\r\n", content_type);
    }

    // 内容长度，支持空body
    char content_length_header[256];
    size_t body_length = body ? strlen(body) : 0;
    snprintf(content_length_header, sizeof(content_length_header), "Content-Length: %zu\r\n", body_length);

    // 连接保持
    const char* connection_header = "Connection: keep-alive\r\n";
    const char* end_of_headers = "\r\n";

    // 构建请求
    size_t offset = 0;
    int n = snprintf(buffer + offset, buffer_size - offset, request_line_template, url_info->route);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at request line\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    n = snprintf(buffer + offset, buffer_size - offset, "%s", host_header);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at host header\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    if (content_type_header[0] != '\0') {
        n = snprintf(buffer + offset, buffer_size - offset, "%s", content_type_header);
        if (n < 0 || (size_t)n >= buffer_size - offset) {
            LOG_E("Buffer overflow detected at content type header\n");
            free_url_info(url_info);
            return -1;
        }
        offset += n;
    }

    n = snprintf(buffer + offset, buffer_size - offset, "%s", content_length_header);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at content length header\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    n = snprintf(buffer + offset, buffer_size - offset, "%s", connection_header);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at connection header\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    n = snprintf(buffer + offset, buffer_size - offset, "%s", end_of_headers);
    if (n < 0 || (size_t)n >= buffer_size - offset) {
        LOG_E("Buffer overflow detected at end of headers\n");
        free_url_info(url_info);
        return -1;
    }
    offset += n;

    // 添加请求体（如果存在）
    if (body && body_length > 0) {
        if (body_length >= buffer_size - offset) {
            LOG_E("Buffer overflow detected at body\n");
            free_url_info(url_info);
            return -1;
        }
        memcpy(buffer + offset, body, body_length);
        offset += body_length;
    }

    // 确保字符串以空字符结尾
    if (offset < buffer_size) {
        buffer[offset] = '\0';
    } else {
        LOG_E("Buffer overflow detected at null-termination\n");
        free_url_info(url_info);
        return -1;
    }

    free_url_info(url_info);
    return 0;
}
