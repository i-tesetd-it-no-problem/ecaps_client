
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "app/app_led.h"
#include "utils/logger.h"
#include "utils/qfsm.h"

// 单个LED设备
struct led_device {
	const char *dev_path; // 设备路径
	int fd;				  // 文件描述符
	uint8_t level;		  // 电平
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
	enum led_state cur_state; // 当前状态
	size_t counter;			  // 计时器
	size_t state_period;	  // 当前状态的闪烁周期
};

// 全局示例
static struct led_task *led_handle = NULL;

// 状态函数
static qstate app_led_normal(qfsm_t *me, qevent_t const *ev); // 周期闪烁状态
static qstate app_led_scroll(qfsm_t *me, qevent_t const *ev); // 跑马灯状态

// 所有状态函数
static qstate_handler _all_state[LED_STATE_MAX] = {
	[LED_STATE_NORMAL] = app_led_normal,
	[LED_STATE_SCROLL] = app_led_scroll,
};

/**
 * @brief 周期闪烁状态处理函数
 * 
 * @param me 状态机指针
 * @param e 事件指针
 * @return qstate 处理结果
 */
static qstate app_led_normal(qfsm_t *me, qevent_t const *e)
{
	struct led_task *p = (struct led_task *)me;

	switch (e->sig) {
	case Q_ENTRY_SIG:
		p->counter = 0;
		p->state_period = 50; // 50ms
		break;

	case Q_APP_EVENT_TIMEOUT:
		p->counter += APP_LED_TASK_PERIOD_MS;
		if (p->counter < p->state_period)
			break;
		p->counter = 0;

		for (enum led_idx i = LED_IDX_1; i < LED_ID_MAX; i++) {
			if (_led_dev[i].fd < 0)
				continue;
			ssize_t ret = write(_led_dev[i].fd, &_led_dev[i].level, 1); // 写入当前电平
			if (ret != 1)
				LOG_E("write to %s failed", _led_dev[i].dev_path);
			_led_dev[i].level ^= 1; // 翻转电平
		}
		break;

	case CHANGE_SIG:
		LOG_I("Changing LED state.");
		return Q_TRAN(_all_state[p->cur_state]); // 状态转移

	default:
		break;
	}

	return Q_EVENT_HANDLED;
}

/**
 * @brief 跑马灯状态处理函数
 * 
 * @param me 状态机指针
 * @param e 事件指针
 * @return qstate 处理结果
 */
static qstate app_led_scroll(qfsm_t *me, qevent_t const *e)
{
	struct led_task *p = (struct led_task *)me;
	static enum led_idx current_led = LED_IDX_1;

	switch (e->sig) {
	case Q_ENTRY_SIG:
		p->counter = 0;
		p->state_period = 25;
		break;

	case Q_APP_EVENT_TIMEOUT:
		p->counter += APP_LED_TASK_PERIOD_MS;
		if (p->counter < p->state_period)
			break;
		p->counter = 0;

		// 熄灭所有LED
		for (enum led_idx i = LED_IDX_1; i < LED_ID_MAX; i++) {
			if (_led_dev[i].fd < 0)
				continue;
			uint8_t off = 0;
			if (write(_led_dev[i].fd, &off, 1) != 1)
				LOG_E("Failed to turn off LED");
		}

		// 点亮当前LED
		if (_led_dev[current_led].fd >= 0) {
			uint8_t on = 1;
			if (write(_led_dev[current_led].fd, &on, 1) != 1)
				LOG_E("Failed to turn on LED");
		}

		// 下一个LED
		current_led = (current_led + 1) % LED_ID_MAX;

		break;

	case CHANGE_SIG:
		LOG_I("Changing LED state.");
		return Q_TRAN(_all_state[p->cur_state]); // 状态转移

	default:
		break;
	}

	return Q_EVENT_HANDLED;
}

/**
 * @brief 初始化LED
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_led_init(void **p_priv)
{
	if (!p_priv)
		return false;

	uint8_t err = 0;

	// LED任务实例
	struct led_task *led_ins = malloc(sizeof(struct led_task));
	if (!led_ins) {
		LOG_E("Malloc led instance failed");
		return false;
	}
	led_ins->cur_state = LED_STATE_SCROLL;

	// 打开所有LED设备
	for (enum led_idx i = LED_IDX_1; i < LED_ID_MAX; i++) {
		int fd = open(_led_dev[i].dev_path, O_WRONLY);
		if (fd < 0) {
			err++;
			LOG_E("open %s failed", _led_dev[i].dev_path);
			continue;
		}
		_led_dev[i].fd = fd;
	}

	if (!err)
		LOG_I("All LEDs initialized successfully.");
	else
		LOG_E("Failed to initialize %d LEDs.", err);

	// 初始化状态机为当前状态
	qevent_t ev = { .sig = Q_EMPTY_SIG };
	qfsm_init(&led_ins->fsm, _all_state[led_ins->cur_state], &ev);

	*p_priv = led_ins; // 保存指针
	led_handle = led_ins;

	return (err == 0) ? true : false;
}

/**
 * @brief 去初始化LED
 * 
 * @param priv 私有数据指针
 */
void app_led_deinit(void *priv)
{
	if (!priv)
		return;

	for (enum led_idx i = LED_IDX_1; i < LED_ID_MAX; i++) {
		if (_led_dev[i].fd >= 0) {
			close(_led_dev[i].fd);
			_led_dev[i].fd = -1;
			LOG_I("Closed LED device: %s", _led_dev[i].dev_path);
		}
	}

	struct led_task *led_ins = priv;
	free(led_ins);
}

/**
 * @brief LED任务
 * 
 * @param priv 私有数据指针
 */
void app_led_task(void *priv)
{
	struct led_task *led_ins = priv;

	qevent_t ev = { .sig = Q_APP_EVENT_TIMEOUT };
	qfsm_dispatch(&led_ins->fsm, &ev);
}

/**
 * @brief 切换LED状态
 * 
 * @param state 目标LED状态
 */
void led_chg_state(enum led_state state)
{
	if (state >= LED_STATE_MAX || state < LED_STATE_NORMAL || !led_handle)
		return;

	led_handle->cur_state = state;

	qevent_t ev = { .sig = CHANGE_SIG };
	qfsm_dispatch(&led_handle->fsm, &ev);

	LOG_I("LED state changed to %d", state);
}
