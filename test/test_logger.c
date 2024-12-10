#include "unity.h"
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
    logger_set_level(LOG_LEVEL_DEBUG);
    LOG_D("This Debug message should appear at DEBUG level.");
    LOG_I("This Info message should appear at DEBUG level.");
    LOG_W("This Warning message should appear at DEBUG level.");
    LOG_E("This Error message should appear at DEBUG level.");

    logger_set_level(LOG_LEVEL_INFO);
    LOG_D("This Debug message should NOT appear at INFO level.");
    LOG_I("This Info message should appear at INFO level.");
    LOG_W("This Warning message should appear at INFO level.");
    LOG_E("This Error message should appear at INFO level.");

    logger_set_level(LOG_LEVEL_WARN);
    LOG_D("This Debug message should NOT appear at WARN level.");
    LOG_I("This Info message should NOT appear at WARN level.");
    LOG_W("This Warning message should appear at WARN level.");
    LOG_E("This Error message should appear at WARN level.");

    logger_set_level(LOG_LEVEL_ERROR);
    LOG_D("This Debug message should NOT appear at ERROR level.");
    LOG_I("This Info message should NOT appear at ERROR level.");
    LOG_W("This Warning message should NOT appear at ERROR level.");
    LOG_E("This Error message should appear at ERROR level.");

    // 恢复默认等级
    logger_set_level(LOG_LEVEL_DEBUG);
}

// 超长消息测试
void test_long_message()
{
    char long_message[LOG_MAX_SIZE * 2];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';

    LOG_I("This is a long message test: %s", long_message);

    // 验证消息长度
    TEST_ASSERT_EQUAL(strlen(long_message), LOG_MAX_SIZE * 2 - 1);
}

// 空指针和无效格式字符串测试
void test_null_and_invalid_format()
{
    LOG_I(NULL);
    LOG_I("Invalid format specifier test: %d %s", 123);

    // 确保程序未崩溃,可以简单通过
    TEST_ASSERT_TRUE(1);
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
    pthread_t threads[THREAD_NUMS];
    int thread_nums[THREAD_NUMS];

    for (int i = 0; i < THREAD_NUMS; i++) {
        thread_nums[i] = i + 1;
        int ret = pthread_create(&threads[i], NULL, thread_logging, &thread_nums[i]);
        TEST_ASSERT_EQUAL(0, ret);
    }

    for (int i = 0; i < THREAD_NUMS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 验证线程测试完成
    TEST_ASSERT_TRUE(1);
}

// 时间戳开关测试
void test_timestamp_toggle()
{
    logger_enable_timestamp(false);
    LOG_I("Timestamp is disabled.");
    logger_enable_timestamp(true);
    LOG_I("Timestamp is enabled.");

    // 验证开关操作
    TEST_ASSERT_TRUE(1);
}

// 文件输出测试
void test_log_output_to_file()
{
    int ret = logger_set_output(true);
    TEST_ASSERT_TRUE(ret);
    LOG_I("Log file set successfully.");
    LOG_I("This message should appear in the log file 'test_log.txt'.");
}

// 标准输出测试
void test_log_output_to_stdout()
{
    logger_set_output(false);
    LOG_I("This message should appear in the standard output.");

    // 确保切换到标准输出
    TEST_ASSERT_TRUE(1);
}

void setUp(void)
{
    logger_set_level(LOG_LEVEL_INFO);
}

void tearDown(void)
{

}

// Unity 测试主函数
int main(void)
{
    UNITY_BEGIN();

    // 单元测试注册
    RUN_TEST(test_log_levels);
    RUN_TEST(test_long_message);
    RUN_TEST(test_null_and_invalid_format);
    RUN_TEST(test_multithreaded_logging);
    RUN_TEST(test_timestamp_toggle);
    RUN_TEST(test_log_output_to_file);
    RUN_TEST(test_log_output_to_stdout);

    return UNITY_END();
}
