#include <signal.h>

#include "utils/epoll_timer.h"
#include "utils/logger.h"

#include "app/main.h"

// LED 任务
static struct epoll_timer_task led_task = {
	.f_init = app_led_init,
	.f_entry = app_led_task,
	.f_deinit = app_led_deinit,
	.period_ms = APP_LED_TASK_PERIOD_MS,
};

// 采集温湿度任务
static struct epoll_timer_task si7006_task = {
	.f_init = app_si7006_init,
	.f_entry = app_si7006_task,
	.f_deinit = app_si7006_deinit,
	.period_ms = APP_SI7006_TASK_PERIOD,
};

// 采集工作电压/电流任务
struct epoll_timer_task coll_i_v_task = {
	.f_init = app_lmv358_init,
	.f_entry = app_lmv358_task,
	.f_deinit = app_lmv358_deinit,
	.period_ms = APP_COLL_I_V_PERIOD,
};

// 数码管任务
struct epoll_timer_task digital_task = {
	.f_init = app_digital_init,
	.f_entry = app_digital_task,
	.f_deinit = app_digital_deinit,
	.period_ms = APP_DIGITAL_TASK_PERIOD,
};

// 蜂鸣器任务
struct epoll_timer_task beep_task = {
	.f_init = app_beep_init,
	.f_entry = NULL,
	.f_deinit = NULL,
	.period_ms = 0,
};

// 小风扇任务
struct epoll_timer_task fan_task = {
	.f_init = app_fan_init,
	.f_entry = NULL,
	.f_deinit = NULL,
	.period_ms = 0,
};

// 马达任务
struct epoll_timer_task motor_task = {
	.f_init = app_motor_init,
	.f_entry = NULL,
	.f_deinit = NULL,
	.period_ms = 0,
};

// 红外/光强/接近 任务
struct epoll_timer_task ap3216c_task = {
	.f_init = app_ap3216c_init,
	.f_entry = app_ap3216c_task,
	.f_deinit = app_ap3216c_deinit,
	.period_ms = APP_AP3216C_TASK_PEIOD,
};

// 心率血氧 任务
struct epoll_timer_task max30102_task = {
	.f_init = app_max30102_init,
	.f_entry = app_max30102_task,
	.f_deinit = app_max30102_deinit,
	.period_ms = APP_MAX30102_TASK_PERIOD,
};

// 服务端上传任务
struct epoll_timer_task upload_task = {
	.f_init = app_upload_init,
	.f_entry = app_upload_task,
	.f_deinit = app_upload_deinit,
	.period_ms = APP_UPLOAD_TASK_PERIOD,
};

struct epoll_timer_task rs485_task = {
	.f_init = app_rs485_init,
	.f_entry = app_rs485_task,
	.f_deinit = app_rs485_deinit,
	.period_ms = APP_RS485_TASK_PERIOD,
};

static volatile et_handle ept = NULL;

static void sigint_handler(int signum)
{
	if (ept)
		epoll_timer_stop(ept); // 退出监听
}

int main(void)
{
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

#ifdef BOARD_ENV
	epoll_timer_add_task(ept, &led_task);
	epoll_timer_add_task(ept, &si7006_task);
	epoll_timer_add_task(ept, &coll_i_v_task);
	epoll_timer_add_task(ept, &digital_task);
	epoll_timer_add_task(ept, &beep_task);
	epoll_timer_add_task(ept, &fan_task);
	epoll_timer_add_task(ept, &motor_task);
	epoll_timer_add_task(ept, &ap3216c_task);
	epoll_timer_add_task(ept, &max30102_task);
	epoll_timer_add_task(ept, &rs485_task);
#else
	epoll_timer_add_task(ept, &upload_task);
#endif

	if (epoll_timer_run(ept) < 0)
		LOG_E("Timer run loop exited with error.");

	epoll_timer_destroy(ept);
	ept = NULL;

	return 0;
}