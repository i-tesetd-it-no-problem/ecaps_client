#include <stdint.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <pthread.h>

#include "utils/logger.h"

#include "app/app_led.h"
#include "app/app_beep.h"
#include "app/app_fans.h"
#include "app/app_vibration_motor.h"
#include "app/app_vol_cur.h"
#include "app/app_digital.h"
#include "app/app_si7006.h"

enum test_cmd {
	TEST_CMD_LED_NORMAL, // 0 正常闪烁
	TEST_CMD_LED_SCROLL, // 1 跑马灯
	TEST_CMD_BEEP_ON,	 // 2 开启蜂鸣器
	TEST_CMD_BEEP_OFF,	 // 3 关闭蜂鸣器
	TEST_CMD_FANS_ON,	 // 4 开启风扇
	TEST_CMD_FANS_OFF,	 // 5 关闭风扇
	TEST_CMD_MOTOR_ON,	 // 6 开启马达
	TEST_CMD_MOTOR_OFF,	 // 7 关闭马达

	TEST_CMD_LED_MAX,
};

void *test(void *arg)
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

		switch (state) {
		case TEST_CMD_LED_NORMAL:
			led_chg_state(LED_STATE_NORMAL);
			break;
		case TEST_CMD_LED_SCROLL:
			led_chg_state(LED_STATE_SCROL);
			break;
		case TEST_CMD_BEEP_ON:
			beep_control(true);
			break;
		case TEST_CMD_BEEP_OFF:
			beep_control(false);
			break;
		case TEST_CMD_FANS_ON:
			fans_control(true);
			break;
		case TEST_CMD_FANS_OFF:
			fans_control(false);
			break;
		case TEST_CMD_MOTOR_ON:
			vibration_motor_control(true);
			break;
		case TEST_CMD_MOTOR_OFF:
			vibration_motor_control(false);
			break;
		default:
			break;
		}
	}
}

/********************主任务********************/

int main()
{
	logger_enable_timestamp(true); // 开启日志时间

	logger_set_level(LOG_LEVEL_INFO); // 设置日志等级

	// 创建LED线程
	pthread_t led_thread;
	int ret_led;
	ret_led = pthread_create(&led_thread, NULL, app_led_task, NULL);
	if (ret_led != 0)
		LOG_E("Failed to create thread");

	// 创建电压电流采集线程
	pthread_t coll_v_i_thread;
	int ret_v_i;
	ret_v_i = pthread_create(&coll_v_i_thread, NULL, coll_v_i_task, NULL);
	if (ret_v_i != 0)
		LOG_E("Failed to create thread");

	// 创建数码管线程
	pthread_t digtal_thread;
	int ret_digtal;
	ret_digtal = pthread_create(&digtal_thread, NULL, app_digital_task, NULL);
	if (ret_digtal != 0)
		LOG_E("Failed to create thread");

	// 创建采集温湿度线程
	pthread_t humi_temp_thread;
	int ret_humi_temp;
	ret_humi_temp = pthread_create(&humi_temp_thread, NULL, app_si7006_task, NULL);
	if (ret_humi_temp != 0)
		LOG_E("Failed to create thread");

	// 创建测试线程
	pthread_t test_thread;
	int ret_test;
	ret_test = pthread_create(&test_thread, NULL, test, NULL);

	app_beep_init();			// 初始化蜂鸣器
	app_fan_init();				// 初始化风扇
	app_vibration_motor_init(); // 初始化振动马达

	if (ret_led == 0)
		pthread_join(led_thread, NULL); // 等待LED线程

	if (ret_v_i == 0)
		pthread_join(coll_v_i_thread, NULL); // 等待LED线程

	if (ret_digtal == 0)
		pthread_join(digtal_thread, NULL); // 等待数码管线程

	if (ret_humi_temp == 0)
		pthread_join(humi_temp_thread, NULL); // 等待温湿度采集线程

	if (ret_test == 0)
		pthread_join(test_thread, NULL); // 等待测试线程

	return 0;
}