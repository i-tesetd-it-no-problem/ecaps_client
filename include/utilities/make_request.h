#ifndef __MAKE_REQUEST_H__
#define __MAKE_REQUEST_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * 构建一个HTTP GET请求字符串。
 *
 * @param url 完整的URL（例如 "https://www.example.com/api/data?id=10&value=test"）
 * @param buffer 用于存储请求的缓冲区
 * @param buffer_size 缓冲区的大小
 * @return 成功返回0，失败返回非零值
 */
int build_get_request(const char* url, char* buffer, size_t buffer_size);

/**
 * 构建一个HTTP POST请求字符串。
 *
 * @param url 完整的URL（例如 "https://www.example.com/api/submit"）
 * @param content_type 内容类型（例如 "application/json"）
 * @param body 请求体内容（例如 "{\"name\":\"John\",\"age\":30}"），可以为NULL表示空请求体
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
#define BUILD_POST(url, buffer, buffer_size) \
    build_post_request(url, "application/x-www-form-urlencoded", NULL, buffer, buffer_size) // 空POST请求

#define BUILD_POST_JSON(url, body, buffer, buffer_size) \
    build_post_request(url, "application/json", body, buffer, buffer_size) // JSON内容类型的POST请求

#endif /* __MAKE_REQUEST_H__ */
