/**
 * @file app_fan.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 蜂鸣器
 * @version 1.0
 * @date 2024-12-03
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

#include "app/app_fan.h"
#include "app/app_pwm.h"

#include "utils/logger.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define PWM_CHIP (4U)	 // PWM 号
#define PWM_CHANNEL (0U) // PWM 通道 1
#define CALC_STEP (10)	 // 步长 10

struct app_fan {
	size_t period;		   // 周期
	size_t duty_cycle;	   // 占空比
	pthread_mutex_t mutex; // 互斥锁
};

static struct app_fan *fan = NULL;

/**
 * @brief 初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_fan_init(void **p_priv)
{
	bool ret = false;

	// 实例化
	struct app_fan *p_fan = malloc(sizeof(struct app_fan));
	if (!p_fan) {
		LOG_E("Malloc vep failed");
		return false;
	}

	int res = pthread_mutex_init(&p_fan->mutex, NULL);
	if (ret != 0) {
		LOG_E("Init mutex failed");
		goto err_free_fan;
	}
	p_fan->period = 1000;
	p_fan->duty_cycle = 500;

	ret = export_pwm(PWM_CHIP, PWM_CHANNEL);
	if (!ret) {
		LOG_E("create fan failed");
		goto err_free_mutex;
	}

	pthread_mutex_lock(&p_fan->mutex);

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, p_fan->period);		   // 设置周期(音调)
	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, p_fan->duty_cycle); // 设置占空比(强度)

	pthread_mutex_unlock(&p_fan->mutex);

	*p_priv = p_fan;
	fan = p_fan;

	return true;

err_free_mutex:
	pthread_mutex_destroy(&fan->mutex);

err_free_fan:
	free(fan);

	return false;
}

/**
 * @brief 开启/关闭
 * 
 * @param enable 
 */
void fan_control(bool enable)
{
	pthread_mutex_lock(&fan->mutex);

	if (!fan)
		return;

	enable_pwm(PWM_CHIP, PWM_CHANNEL, enable);

	pthread_mutex_unlock(&fan->mutex);
}

/**
 * @brief 提高占空比
 * 
 */
void app_fan_add_duty(size_t duty)
{
	if (!fan)
		return;

	pthread_mutex_lock(&fan->mutex);

	if (MAX_FAN_DUTY - duty < fan->duty_cycle)
		fan->duty_cycle = MAX_FAN_DUTY;
	else
		fan->duty_cycle += duty;

	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, fan->duty_cycle);

	pthread_mutex_unlock(&fan->mutex);
}

/**
 * @brief 减少占空比
 * 
 */
void app_fan_sub_duty(size_t duty)
{
	if (!fan)
		return;

	pthread_mutex_lock(&fan->mutex);

	if (fan->duty_cycle < duty)
		fan->duty_cycle = 0;
	else
		fan->duty_cycle -= duty;

	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, fan->duty_cycle);

	pthread_mutex_unlock(&fan->mutex);
}

/**
 * @brief 提高周期
 * 
 */
void app_fan_add_period(size_t period)
{
	if (!fan)
		return;

	pthread_mutex_lock(&fan->mutex);

	if (MAX_FAN_DUTY - period < fan->period)
		fan->period = MAX_FAN_PERIOD;
	else
		fan->period += period;

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, fan->period);

	pthread_mutex_unlock(&fan->mutex);
}

/**
 * @brief 减少周期
 * 
 */
void app_fan_sub_period(size_t period)
{
	if (!fan)
		return;

	pthread_mutex_lock(&fan->mutex);

	if (fan->period < period)
		fan->period = 0;
	else
		fan->period -= period;

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, fan->period);

	pthread_mutex_unlock(&fan->mutex);
}
