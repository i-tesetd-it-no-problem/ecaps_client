#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "app/app_led.h"
#include "utils/logger.h"
#include "utils/qfsm.h"

// 单个LED设备
struct led_device {
	char *dev_path; // 设备名
	int fd;			// 文件描述符
	uint8_t level;	// 电平
};

// LED个数
enum led_idx {
	LED_IDX_1,
	LED_IDX_2,
	LED_IDX_3,
	LED_IDX_4,
	LED_IDX_5,
	LED_IDX_6,

	LED_ID_MAX,
};

// 所有的LED设备
static struct led_device _led_dev[LED_ID_MAX] = {
	[LED_IDX_1] = { "/dev/led1", -1, .level = 0 },
	[LED_IDX_2] = { "/dev/led2", -1, .level = 0 },
	[LED_IDX_3] = { "/dev/led3", -1, .level = 0 },
	[LED_IDX_4] = { "/dev/led4", -1, .level = 0 },
	[LED_IDX_5] = { "/dev/led5", -1, .level = 0 },
	[LED_IDX_6] = { "/dev/led6", -1, .level = 0 },
};

// 自定义信号
enum app_led_signal {
	CHANGE_SIG = Q_APP_EVENT_TIMEOUT + 1, // 切换LED状态
};

// 任务实例
struct led_task {
	qfsm_t fsm;				  // 状态机
	pthread_mutex_t mutex;	  // 互斥锁
	bool running;			  // 运行标志
	enum led_state cur_state; // 当前状态
};

static struct led_task led_ctor = {
	.cur_state = LED_STATE_NORMAL,
	.running = true,
	.mutex = PTHREAD_MUTEX_INITIALIZER,
};

static qstate app_led_normal(qfsm_t *me, qevent_t const *ev); // 周期闪烁状态
static qstate app_led_scroll(qfsm_t *me, qevent_t const *ev); // 跑马灯状态

// 所有状态
static qstate_handler _all_state[LED_STATE_NONE] = {
	[LED_STATE_NORMAL] = app_led_normal,
	[LED_STATE_SCROL] = app_led_scroll,
};

static void app_led_init(void)
{
	uint8_t err = 0;
	static bool inited = false;
	if (inited)
		return;
	inited = true;

	pthread_mutex_lock(&led_ctor.mutex);
	// 打开所有LED
	for (enum led_idx i = LED_IDX_1; i < LED_ID_MAX; i++) {
		int fd = open(_led_dev[i].dev_path, O_WRONLY);
		if (fd < 0) {
			err++;
			LOG_E("open %s failed", _led_dev[i].dev_path);
			continue;
		}
		_led_dev[i].fd = fd;
	}
	pthread_mutex_unlock(&led_ctor.mutex);

	if (!err)
		LOG_I("all led init success");

	qevent_t ev = { .sig = Q_EMPTY_SIG };
	qfsm_init(&led_ctor.fsm, app_led_normal, &ev); // 初始化为周期闪烁状态
}

// 复位LED
static void app_led_deinit(void)
{
	pthread_mutex_lock(&led_ctor.mutex);
	for (enum led_idx i = LED_IDX_1; i < LED_ID_MAX; i++) {
		if (_led_dev[i].fd >= 0) {
			close(_led_dev[i].fd);
			_led_dev[i].fd = -1;
		}
	}
	pthread_mutex_unlock(&led_ctor.mutex);
}

// 周期闪烁
qstate app_led_normal(qfsm_t *me, qevent_t const *e)
{
	struct led_task *p = (struct led_task *)me;
	struct timespec req;
	uint32_t period = 200; // 200ms

	switch (e->sig) {
	case Q_APP_EVENT_TIMEOUT:

		req.tv_sec = period / 1000;
		req.tv_nsec = (period % 1000) * 1000000L;

		nanosleep(&req, NULL); // 休眠

		for (enum led_idx i = LED_IDX_1; i < LED_ID_MAX; i++) {
			if (_led_dev[i].fd < 0)
				continue;
			pthread_mutex_lock(&led_ctor.mutex);
			ssize_t ret = write(_led_dev[i].fd, &_led_dev[i].level, 1); // 写入当前电平
			if (ret != 1)
				LOG_E("write to %s failed", _led_dev[i].dev_path);
			_led_dev[i].level ^= 1; // 翻转
			pthread_mutex_unlock(&led_ctor.mutex);
		}
		break;

	case CHANGE_SIG:
		LOG_I("Change Status");
		return Q_TRAN(_all_state[p->cur_state]); // 状态转移
	default:
		break;
	}

	return Q_EVENT_HANDLED;
}

// 跑马灯
qstate app_led_scroll(qfsm_t *me, qevent_t const *e)
{
	struct led_task *p = (struct led_task *)me;
	static enum led_idx current_led = LED_IDX_1; //
	struct timespec req;

	switch (e->sig) {
	case Q_APP_EVENT_TIMEOUT:
		req.tv_sec = 0;
		req.tv_nsec = 50 * 1000000L; // 50 毫秒切换

		pthread_mutex_lock(&led_ctor.mutex);

		// 熄灭所有 LED
		for (enum led_idx i = LED_IDX_1; i < LED_ID_MAX; i++) {
			if (_led_dev[i].fd < 0)
				continue;
			uint8_t off = 0;
			if (write(_led_dev[i].fd, &off, 1) != 1)
				LOG_E("Failed to turn off LED %s", _led_dev[i].dev_path);
		}

		// 点亮当前 LED
		if (_led_dev[current_led].fd >= 0) {
			uint8_t on = 1;
			if (write(_led_dev[current_led].fd, &on, 1) != 1)
				LOG_E("Failed to turn on LED %s", _led_dev[current_led].dev_path);
		}

		pthread_mutex_unlock(&led_ctor.mutex);

		// 下一个 LED
		current_led = (current_led + 1) % LED_ID_MAX;

		// 休眠
		nanosleep(&req, NULL);
		break;

	case CHANGE_SIG:
		return Q_TRAN(_all_state[p->cur_state]); // 状态转移

	default:
		break;
	}

	return Q_EVENT_HANDLED;
}

/***************************API***************************/

/**
 * @brief LED 任务线程 pthread函数指针形式
 * 
 * @param arg 线程参数
 * @return void* 返回 NULL
 */
void *app_led_task(void *arg)
{
	app_led_init();

	while (led_ctor.running) {
		qevent_t ev = { .sig = Q_APP_EVENT_TIMEOUT };
		qfsm_dispatch(&led_ctor.fsm, &ev);
	}

	app_led_deinit();

	return NULL;
}

/**
 * @brief 停止LED任务
 * 
 */
void stop_led_task(void)
{
	led_ctor.running = false;
}

/**
 * @brief 切换LED状态
 * 
 * @param state 
 */
void led_chg_state(enum led_state state)
{
	if (state >= LED_STATE_NONE || state < LED_STATE_NORMAL)
		return;

	qevent_t ev = { .sig = CHANGE_SIG };
	led_ctor.cur_state = state;
	qfsm_dispatch(&led_ctor.fsm, &ev);
}