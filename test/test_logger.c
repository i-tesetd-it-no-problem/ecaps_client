#include "utils/logger.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "user_config.h"

#define THREAD_NUMS 10

// 单一日志等级测试
void test_log_levels()
{
	printf("\n=== Testing log levels individually ===\n");

	printf("\nTesting LOG_LEVEL_DEBUG:\n");
	logger_set_level(LOG_LEVEL_DEBUG);
	LOG_D("This Debug message should appear at DEBUG level.");
	LOG_I("This Info message should appear at DEBUG level.");
	LOG_W("This Warning message should appear at DEBUG level.");
	LOG_E("This Error message should appear at DEBUG level.");

	printf("\nTesting LOG_LEVEL_INFO:\n");
	logger_set_level(LOG_LEVEL_INFO);
	LOG_D("This Debug message should NOT appear at INFO level.");
	LOG_I("This Info message should appear at INFO level.");
	LOG_W("This Warning message should appear at INFO level.");
	LOG_E("This Error message should appear at INFO level.");

	printf("\nTesting LOG_LEVEL_WARN:\n");
	logger_set_level(LOG_LEVEL_WARN);
	LOG_D("This Debug message should NOT appear at WARN level.");
	LOG_I("This Info message should NOT appear at WARN level.");
	LOG_W("This Warning message should appear at WARN level.");
	LOG_E("This Error message should appear at WARN level.");

	printf("\nTesting LOG_LEVEL_ERROR:\n");
	logger_set_level(LOG_LEVEL_ERROR);
	LOG_D("This Debug message should NOT appear at ERROR level.");
	LOG_I("This Info message should NOT appear at ERROR level.");
	LOG_W("This Warning message should NOT appear at ERROR level.");
	LOG_E("This Error message should appear at ERROR level.");

	// 还原到 DEBUG 级别
	logger_set_level(LOG_LEVEL_DEBUG);
}

// 超长消息测试
void test_long_message()
{
	printf("\n=== Testing long log message ===\n");

	char long_message[LOG_MAX_SIZE * 2];
	memset(long_message, 'A', sizeof(long_message) - 1);
	long_message[sizeof(long_message) - 1] = '\0';

	LOG_I("This is a long message test: %s", long_message);
}

// 空指针和无效格式字符串测试
void test_null_and_invalid_format()
{
	printf("\n=== Testing null pointers and invalid formats ===\n");

	LOG_I(NULL);
	LOG_I("Invalid format specifier test: %d %s", 123);
}

// 线程日志函数
void *thread_logging(void *arg)
{
	int thread_num = *(int *)arg;
	for (int i = 0; i < 5; i++) {
		LOG_D("Thread %d, Debug message %d", thread_num, i);
		LOG_I("Thread %d, Info message %d", thread_num, i);
		LOG_W("Thread %d, Warning message %d", thread_num, i);
		LOG_E("Thread %d, Error message %d", thread_num, i);
		usleep(100000); // 100ms
	}
	return NULL;
}

// 多线程测试
void test_multithreaded_logging()
{
	printf("\n=== Testing multi-threaded logging output ===\n");

	pthread_t threads[THREAD_NUMS];
	int thread_nums[THREAD_NUMS];

	for (int i = 0; i < THREAD_NUMS; i++) {
		thread_nums[i] = i + 1;
		if (pthread_create(&threads[i], NULL, thread_logging, &thread_nums[i]) != 0) {
			perror("Failed to create thread");
			exit(1);
		}
	}

	for (int i = 0; i < THREAD_NUMS; i++)
		pthread_join(threads[i], NULL);
}

// 时间戳开关测试
void test_timestamp_toggle()
{
	printf("\n=== Testing timestamp toggle ===\n");
	logger_enable_timestamp(false);
	LOG_I("Timestamp is disabled.");
	logger_enable_timestamp(true);
	LOG_I("Timestamp is enabled.");
}

// 文件输出测试
void test_log_output_to_file()
{
	printf("\n=== Testing log output to file ===\n");
	if (logger_set_output(true)) {
		LOG_I("Log file set successfully.");
		LOG_I("This message should appear in the log file 'test_log.txt'.");
	} else
		fprintf(stderr, "Failed to set log file.\n");
}

// 标准输出测试
void test_log_output_to_stdout()
{
	printf("\n=== Testing log output to stdout ===\n");
	logger_set_output(false);
	LOG_I("This message should appear in the standard output.");
}

// 综合测试
void run_tests(bool to_file)
{
	if (to_file)
		test_log_output_to_file();
	else
		test_log_output_to_stdout();

	test_log_levels();
	test_timestamp_toggle();
	test_long_message();
	test_null_and_invalid_format();
	test_multithreaded_logging();
}

int main()
{
	printf("\n=== Running tests to standard output ===\n");
	run_tests(false); // 测试标准输出

	printf("\n=== Running tests to log file ===\n");
	run_tests(true); // 测试文件输出

	printf("\n=== Test complete ===\n");
	return 0;
}
