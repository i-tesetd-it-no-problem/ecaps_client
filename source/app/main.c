/**
 * @file main.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 主任务入口
 * @version 1.0
 * @date 2024-12-11
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

#include <signal.h>

#include "utils/logger.h"
#include "utils/epoll_timer.h"

#include "app/app_beep.h"
#include "app/app_digital.h"
#include "app/app_fan.h"
#include "app/app_led.h"
#include "app/app_si7006.h"
#include "app/app_motor.h"
#include "app/app_lmv358.h"
#include "app/app_ap3216c.h"
#include "app/app_max30102.h"
#include "app/app_upload.h"
#include "app/app_rs485.h"
#include "app/app_can_task.h"
#include "app/app_rs485_master.h"

// LED 任务
static const struct epoll_timer_task led_task = {
	.task_name = "led task",
	.f_init = app_led_init,
	.f_entry = app_led_task,
	.f_deinit = app_led_deinit,
	.period_ms = APP_LED_TASK_PERIOD_MS,
};

// 采集温湿度任务
static const struct epoll_timer_task si7006_task = {
	.task_name = "si7006 task",
	.f_init = app_si7006_init,
	.f_entry = app_si7006_task,
	.f_deinit = app_si7006_deinit,
	.period_ms = APP_SI7006_TASK_PERIOD,
};

// 采集工作电压/电流任务
static const struct epoll_timer_task lmv358_task = {
	.task_name = "lmv358 task",
	.f_init = app_lmv358_init,
	.f_entry = app_lmv358_task,
	.f_deinit = app_lmv358_deinit,
	.period_ms = APP_COLL_I_V_PERIOD,
};

// 数码管任务
static const struct epoll_timer_task digital_task = {
	.task_name = "digital task",
	.f_init = app_digital_init,
	.f_entry = app_digital_task,
	.f_deinit = app_digital_deinit,
	.period_ms = APP_DIGITAL_TASK_PERIOD,
};

// 蜂鸣器任务
static const struct epoll_timer_task beep_task = {
	.task_name = "beep task",
	.f_init = app_beep_init,
	.f_entry = NULL,
	.f_deinit = NULL,
	.period_ms = 0,
};

// 小风扇任务
static const struct epoll_timer_task fan_task = {
	.task_name = "fan task",
	.f_init = app_fan_init,
	.f_entry = NULL,
	.f_deinit = NULL,
	.period_ms = 0,
};

// 马达任务
static const struct epoll_timer_task motor_task = {
	.task_name = "motor task",
	.f_init = app_motor_init,
	.f_entry = NULL,
	.f_deinit = NULL,
	.period_ms = 0,
};

// 红外/光强/接近 任务
static const struct epoll_timer_task ap3216c_task = {
	.task_name = "ap3216c task",
	.f_init = app_ap3216c_init,
	.f_entry = app_ap3216c_task,
	.f_deinit = app_ap3216c_deinit,
	.period_ms = APP_AP3216C_TASK_PEIOD,
};

// 心率血氧 任务
static const struct epoll_timer_task max30102_task = {
	.task_name = "max30102 task",
	.f_init = app_max30102_init,
	.f_entry = app_max30102_task,
	.f_deinit = app_max30102_deinit,
	.period_ms = APP_MAX30102_TASK_PERIOD,
};

// 服务端上传任务
static const struct epoll_timer_task upload_task = {
	.task_name = "upload task",
	.f_init = app_upload_init,
	.f_entry = app_upload_task,
	.f_deinit = app_upload_deinit,
	.period_ms = APP_UPLOAD_TASK_PERIOD,
};

static const struct epoll_timer_task rs485_task = {
	.task_name = "rs485 task",
	.f_init = app_rs485_init,
	.f_entry = app_rs485_task,
	.f_deinit = app_rs485_deinit,
	.period_ms = APP_RS485_TASK_PERIOD,
};

static const struct epoll_timer_task can_task = {
	.task_name = "can task",
	.f_init = app_can_init,
	.f_entry = app_can_task,
	.f_deinit = app_can_deinit,
	.period_ms = APP_CAN_TASK_PERIOD,
};

static const struct epoll_timer_task rs485_master_task = {
	.task_name = "rs485 master task",
	.f_init = app_rs485_master_init,
	.f_entry = app_rs485_master_task,
	.f_deinit = app_rs485_master_deinit,
	.period_ms = APP_RS485_TASK_MASTER_PERIOD,
};

static volatile et_handle ept = NULL; // 监听句柄

// 信号处理函数
static void sigint_handler(int signum)
{
	if (ept)
		epoll_timer_stop(ept); // 退出监听
}

/**
 * @brief 函数入口
 * 
 * @param argc 参数个数
 * @param argv 参数内容
 * @return int 
 */
int main(int argc, const char *argv[])
{
	if (argc != 1) {
		LOG_E("Cant input arguments");
		return -1;
	}

	// 创建监听实例
	ept = epoll_timer_create();
	if (!ept) {
		LOG_E("Failed to create epoll timer.");
		return -1;
	}

	// 设置 Kill 信号 清理资源
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	// 添加所有周期任务待监听
#ifdef BOARD_ENV
	epoll_timer_add_task(ept, &led_task);
	// epoll_timer_add_task(ept, &si7006_task);
	// epoll_timer_add_task(ept, &lmv358_task);
	// epoll_timer_add_task(ept, &digital_task);
	// epoll_timer_add_task(ept, &beep_task);
	// epoll_timer_add_task(ept, &fan_task);
	// epoll_timer_add_task(ept, &motor_task);
	// epoll_timer_add_task(ept, &ap3216c_task);
	// epoll_timer_add_task(ept, &max30102_task);
	// epoll_timer_add_task(ept, &rs485_task);
	epoll_timer_add_task(ept, &rs485_master_task);

	// epoll_timer_add_task(ept, &can_task);

#else
	epoll_timer_add_task(ept, &upload_task);
#endif

	// 轮训监听
	if (epoll_timer_run(ept) < 0)
		LOG_E("Timer run loop exited with error.");

	// 释放监听句柄
	epoll_timer_destroy(ept);
	ept = NULL;

	return 0;
}