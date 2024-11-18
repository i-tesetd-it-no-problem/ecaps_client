#include "unity.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssl/ssl_client.h"
#include "utils/logger.h"

#define MAX_CONCURRENT_REQUESTS 100 // 100个并发请求
#define SERVER_URL "https://49.51.40.135:8001" // 服务器URL
#define GET_URL "https://49.51.40.135:8001/test" // GET 请求URL
#define POST_URL "https://49.51.40.135:8001/submit" // POST 请求URL

#define JSON_BODY "{ \"param1\": \"Hello\", \"param2\": \"World\" }"

// 测试的请求响应缓冲区大小
#define RESPONSE_BUFFER_SIZE (1024 * 2)

// 统计请求结果
static int total_requests = 0;
static int successful_requests = 0;
static int failed_requests = 0;

// SSL客户端测试线程结构体
typedef struct {
    ssl_handle client;
    unsigned char *response_body;
    size_t response_buffer_size;
    size_t *response_body_len;
} test_thread_arg_t;

// 打印测试结果
void log_test_result() {
    LOG_I("Total requests: %d", total_requests);
    LOG_I("Successful requests: %d", successful_requests);
    LOG_I("Failed requests: %d", failed_requests);

    TEST_ASSERT(total_requests > 0);
    TEST_ASSERT(successful_requests > 0);
}

// 模拟SSL请求的线程函数
void *ssl_client_test(void *arg) {
    test_thread_arg_t *test_arg = (test_thread_arg_t *)arg;
    ssl_handle client = test_arg->client;
    unsigned char *response_body = test_arg->response_body;
    size_t response_buffer_size = test_arg->response_buffer_size;
    size_t *response_body_len = test_arg->response_body_len;

    total_requests++;

    int request_type = rand() % 2; // 随机选择 GET 或 POST 请求
    int ret = (request_type == 0)
                  ? ssl_client_get(client, GET_URL, response_body, response_buffer_size, response_body_len)
                  : ssl_client_post(client, POST_URL, JSON_BODY, response_body, response_buffer_size, response_body_len);

    if (ret == 0) 
        successful_requests++;
    else 
        failed_requests++;

    LOG_I("successful_requests: %d, failed_requests: %d", successful_requests, failed_requests);

    return NULL;
}

// 多线程SSL压力测试
void test_ssl_client_stress() {
    pthread_t threads[MAX_CONCURRENT_REQUESTS];
    test_thread_arg_t thread_args[MAX_CONCURRENT_REQUESTS];

    ssl_handle client;
    unsigned char response_body[RESPONSE_BUFFER_SIZE];
    size_t response_body_len;

    // 初始化SSL客户端
    int ret = ssl_client_init(&client);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ssl_client_connect(client, SERVER_URL);
    TEST_ASSERT_EQUAL(0, ret);

    // 创建多个线程发起并发请求
    for (int i = 0; i < MAX_CONCURRENT_REQUESTS; i++) {
        thread_args[i].client = client;
        thread_args[i].response_body = response_body;
        thread_args[i].response_buffer_size = RESPONSE_BUFFER_SIZE;
        thread_args[i].response_body_len = &response_body_len;

        ret = pthread_create(&threads[i], NULL, ssl_client_test, (void *)&thread_args[i]);
        TEST_ASSERT_EQUAL(0, ret);
    }

    // 等待所有线程完成
    for (int i = 0; i < MAX_CONCURRENT_REQUESTS; i++) {
        pthread_join(threads[i], NULL);
    }

    log_test_result();

    // 关闭连接并释放资源
    ssl_client_close(client);
    ssl_client_free(client);
}

// 初始化测试环境
void setUp() {
    srand(time(NULL));
    logger_set_level(LOG_LEVEL_INFO);
    LOG_I("Starting SSL client stress test...");
}

// 清理测试环境
void tearDown() {
    LOG_I("Finished SSL client stress test.");
}

// Unity测试主函数
int main() {
    UNITY_BEGIN();

    RUN_TEST(test_ssl_client_stress);

    return UNITY_END();
}
