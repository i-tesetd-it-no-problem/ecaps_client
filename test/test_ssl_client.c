#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ssl/ssl_client.h"
#include "utilities/logger.h"

#define MAX_CONCURRENT_REQUESTS 100 // 100个并发请求
#define SERVER_URL "https://49.51.40.135:8001" // 服务器URL
#define GET_URL "https://49.51.40.135:8001/test" // GET 请求URL
#define POST_URL "https://49.51.40.135:8001/submit" // POST 请求URL

#define JSON_BODY "{ \"param1\": \"Hello\", \"param2\": \"World\" }"

// 测试的请求响应缓冲区大小
#define RESPONSE_BUFFER_SIZE (1024 * 2)

// SSL客户端测试线程结构体
typedef struct {
	ssl_handle client; // SSL客户端句柄
	unsigned char *response_body; // 请求响应体数据
	size_t response_buffer_size; // 请求响应体数据缓冲区大小
	size_t *response_body_len; // 请求响应体数据长度
} test_thread_arg_t;

// 测试的请求数
static int total_requests = 0; // 总请求数
static int successful_requests = 0; // 成功请求数
static int failed_requests = 0; // 失败请求数
static int remaining_requests = MAX_CONCURRENT_REQUESTS; // 剩余请求数

// 测试结果
void log_test_result()
{
	LOG_I("Total requests: %d\n", total_requests);
	LOG_I("Successful requests: %d\n", successful_requests);
	LOG_I("Failed requests: %d\n", failed_requests);
}

// 线程发起请求
void *ssl_client_test(void *arg)
{
	test_thread_arg_t *test_arg = (test_thread_arg_t *)arg;
	ssl_handle client = test_arg->client;
	unsigned char *response_body = test_arg->response_body;
	size_t response_buffer_size = test_arg->response_buffer_size;
	size_t *response_body_len = test_arg->response_body_len;

	total_requests++;

	// 随机选择 GET 或 POST 请求
	int request_type = rand() % 2; // 0: GET, 1: POST

	int ret = -1;
	if (request_type == 0)
		ret = ssl_client_get(client, GET_URL, response_body, response_buffer_size,
				     response_body_len);
	else
		ret = ssl_client_post(client, POST_URL, JSON_BODY, response_body,
				      response_buffer_size, response_body_len);

	if (ret == 0)
		successful_requests++;
	else
		failed_requests++;

	LOG_I("remaining_requests: %d\n", --remaining_requests);

	return NULL;
}

// 测试入口函数
void ssl_client_stress_test(int concurrent_requests)
{
	pthread_t threads[concurrent_requests];
	test_thread_arg_t thread_args[concurrent_requests];

	ssl_handle client;
	unsigned char response_body[RESPONSE_BUFFER_SIZE];
	size_t response_body_len;

	// 初始化SSL客户端
	int ret = ssl_client_init(&client);
	if (ret != 0) {
		LOG_E("Failed to initialize SSL client.\n");
		return;
	}

	ret = ssl_client_connect(client, SERVER_URL); // 连接服务器
	if (ret != 0) {
		LOG_E("Failed to connect to server");
		ssl_client_free(client);
		return;
	}

	// 创建请求线程
	for (int i = 0; i < concurrent_requests; i++) {
		thread_args[i].client = client;
		thread_args[i].response_body = response_body;
		thread_args[i].response_buffer_size = RESPONSE_BUFFER_SIZE;
		thread_args[i].response_body_len = &response_body_len;

		ret = pthread_create(&threads[i], NULL, ssl_client_test, (void *)&thread_args[i]);
		if (ret != 0) {
			LOG_E("Failed to create thread %d\n", i);
		}
	}

	// 等待所有线程完成
	for (int i = 0; i < concurrent_requests; i++)
		pthread_join(threads[i], NULL);

	log_test_result();

	// 关闭连接并退出
	ssl_client_close(client);
	ssl_client_free(client);
}

int main()
{
	srand(time(NULL)); // 随机数种子

	ssl_client_stress_test(MAX_CONCURRENT_REQUESTS);

	return 0;
}
