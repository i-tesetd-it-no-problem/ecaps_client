/**
 * @file app_beep.c
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

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "utils/logger.h"

#include "app/app_pwm.h"
#include "app/app_beep.h"

#define PWM_CHIP (0U)	 // PWM 号
#define PWM_CHANNEL (0U) // PWM 通道 1
#define CALC_STEP (10)	 // 步长 10

struct app_beep {
	size_t period;		   // 周期
	size_t duty_cycle;	   // 占空比
	pthread_mutex_t mutex; // 互斥锁
};

static struct app_beep *beep = NULL;

/**
 * @brief 初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_beep_init(void **p_priv)
{
	bool ret = false;

	// 实例化
	struct app_beep *p_beep = malloc(sizeof(struct app_beep));
	if (!p_beep) {
		LOG_E("Malloc vep failed");
		return false;
	}

	int res = pthread_mutex_init(&p_beep->mutex, NULL);
	if (ret != 0) {
		LOG_E("Init mutex failed");
		goto err_free_beep;
	}
	p_beep->period = 1000;
	p_beep->duty_cycle = 500;

	ret = export_pwm(PWM_CHIP, PWM_CHANNEL);
	if (!ret) {
		LOG_E("create beep failed");
		goto err_free_mutex;
	}

	pthread_mutex_lock(&p_beep->mutex);

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, p_beep->period);		   // 设置周期(音调)
	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, p_beep->duty_cycle); // 设置占空比(强度)

	pthread_mutex_unlock(&p_beep->mutex);

	*p_priv = p_beep;
	beep = p_beep;

	return true;

err_free_mutex:
	pthread_mutex_destroy(&beep->mutex);

err_free_beep:
	free(beep);

	return false;
}

/**
 * @brief 开启/关闭
 * 
 * @param enable 
 */
void beep_control(bool enable)
{
	pthread_mutex_lock(&beep->mutex);

	if (!beep)
		return;

	enable_pwm(PWM_CHIP, PWM_CHANNEL, enable);

	pthread_mutex_unlock(&beep->mutex);
}

/**
 * @brief 提高占空比
 * 
 */
void app_beep_add_duty(size_t duty)
{
	if (!beep)
		return;

	pthread_mutex_lock(&beep->mutex);

	if (MAX_BEEP_DUTY - duty < beep->duty_cycle)
		beep->duty_cycle = MAX_BEEP_DUTY;
	else
		beep->duty_cycle += duty;

	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, beep->duty_cycle);

	pthread_mutex_unlock(&beep->mutex);
}

/**
 * @brief 减少占空比
 * 
 */
void app_beep_sub_duty(size_t duty)
{
	if (!beep)
		return;

	pthread_mutex_lock(&beep->mutex);

	if (beep->duty_cycle < duty)
		beep->duty_cycle = 0;
	else
		beep->duty_cycle -= duty;

	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, beep->duty_cycle);

	pthread_mutex_unlock(&beep->mutex);
}

/**
 * @brief 提高周期
 * 
 */
void app_beep_add_period(size_t period)
{
	if (!beep)
		return;

	pthread_mutex_lock(&beep->mutex);

	if (MAX_BEEP_DUTY - period < beep->period)
		beep->period = MAX_BEEP_PERIOD;
	else
		beep->period += period;

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, beep->period);

	pthread_mutex_unlock(&beep->mutex);
}

/**
 * @brief 减少周期
 * 
 */
void app_beep_sub_period(size_t period)
{
	if (!beep)
		return;

	pthread_mutex_lock(&beep->mutex);

	if (beep->period < period)
		beep->period = 0;
	else
		beep->period -= period;

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, beep->period);

	pthread_mutex_unlock(&beep->mutex);
}
