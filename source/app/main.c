#include <stdint.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <pthread.h>

#include "utils/logger.h"
#include "app/app_led.h"

void *test_led(void *arg)
{
	while (1) {
		int state;
		LOG_I("input state");
		if (scanf("%d", &state) != 1) {
			LOG_E("Invalid input. Please enter an integer.");
			while (getchar() != '\n')
				; // 清空输入缓冲区
			continue;
		}
		led_chg_state(state);
	}
}

int main()
{
	logger_enable_timestamp(true);	  // 开启时间
	logger_set_level(LOG_LEVEL_INFO); // 日志设置为信息等级

	pthread_t led_thread, led_test_thread;
	int ret_led, led_test;

	// 创建任务

	// 创建LED任务线程
	ret_led = pthread_create(&led_thread, NULL, app_led_task, NULL);
	if (ret_led != 0)
		LOG_E("Failed to create LED thread: %s", strerror(ret_led));

	// 创建LED测试任务线程
	led_test = pthread_create(&led_test_thread, NULL, test_led, NULL);
	if (led_test != 0)
		LOG_E("Failed to create LED thread: %s", strerror(led_test));

	// 运行所有线程
	if (ret_led == 0)
		pthread_join(led_thread, NULL);

	if (led_test == 0)
		pthread_join(led_test_thread, NULL);

	return 0;
}