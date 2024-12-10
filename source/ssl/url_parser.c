/**
 * @file url_parser.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief URL解析
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ssl/url_parser.h"

// URL解析函数
url_info_t* parse_url(const char *url)
{
    if (url == NULL) return NULL;

    url_info_t *info = (url_info_t *)malloc(sizeof(url_info_t));
    if (!info) return NULL;
    memset(info, 0, sizeof(url_info_t));

    const char *url_ptr = url;

    // 解析协议
    const char *protocol_end = strstr(url_ptr, "://");
    if (protocol_end) {
        size_t protocol_len = protocol_end - url_ptr;
        info->protocol = strndup(url_ptr, protocol_len);
        url_ptr = protocol_end + 3; // 跳过 "://"
    } else {
        // 默认使用 "http"
        info->protocol = strdup("http");
    }

    // 将协议转换为小写
    for (char *p = info->protocol; *p; ++p) *p = tolower(*p);

    // 根据协议设置默认端口
    if (strcmp(info->protocol, "http") == 0) 
        info->port = strdup("80");
    else if (strcmp(info->protocol, "https") == 0) 
        info->port = strdup("443");
    else {
        // 不支持的协议
        free(info->protocol);
        free(info);
        return NULL;
    }

    // 解析主机和端口
    const char *host_start = url_ptr;
    const char *host_end = strpbrk(host_start, ":/"); // ':' 或 '/' 或字符串结束
    if (!host_end) {
        // 只有主机
        info->host = strdup(host_start);
        info->route = strdup("/");
        return info;
    } else {
        size_t host_len = host_end - host_start;
        info->host = strndup(host_start, host_len);
        url_ptr = host_end;
    }

    // 检查是否指定了端口
    if (*url_ptr == ':') {
        url_ptr++; // 跳过 ':'
        const char *port_start = url_ptr;
        const char *port_end = strpbrk(port_start, "/");
        if (!port_end) {
            // 只有端口,没有路径
            free(info->port);
            info->port = strdup(port_start);
            info->route = strdup("/");
            return info;
        } else {
            size_t port_len = port_end - port_start;
            free(info->port);
            info->port = strndup(port_start, port_len);
            url_ptr = port_end;
        }
    }

    // 解析路径
    if (*url_ptr == '/') 
        info->route = strdup(url_ptr);
    else 
        info->route = strdup("/");

    // 处理路径中的重复 '/'
    char *normalized_path = (char *)malloc(strlen(info->route) + 1);
    if (!normalized_path) {
        free(info->protocol);
        free(info->host);
        free(info->port);
        free(info->route);
        free(info);
        return NULL;
    }

    char *src = info->route;
    char *dst = normalized_path;
    while (*src) {
        *dst++ = *src;
        if (*src == '/') 
            while (*src == '/') src++;
        else 
            src++;
    }
    *dst = '\0';

    free(info->route);
    info->route = normalized_path;

    return info;
}

void free_url_info(url_info_t *info)
{
    if (!info) return;
    if (info->protocol) free(info->protocol);
    if (info->host) free(info->host);
    if (info->port) free(info->port);
    if (info->route) free(info->route);
    free(info);
}