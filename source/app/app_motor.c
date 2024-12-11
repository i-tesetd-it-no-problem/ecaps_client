/**
 * @file app_motor.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 振动马达
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
#include "app/app_motor.h"

#define PWM_CHIP (8U)	 // PWM 号
#define PWM_CHANNEL (0U) // PWM 通道 1
#define CALC_STEP (10)	 // 步长 10

struct app_motor {
	size_t period;		   // 周期
	size_t duty_cycle;	   // 占空比
	pthread_mutex_t mutex; // 互斥锁
};

static struct app_motor *motor = NULL;

/**
 * @brief 初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_motor_init(void **p_priv)
{
	bool ret = false;

	// 实例化
	struct app_motor *p_motor = malloc(sizeof(struct app_motor));
	if (!p_motor) {
		LOG_E("Malloc vep failed");
		return false;
	}

	int res = pthread_mutex_init(&p_motor->mutex, NULL);
	if (ret != 0) {
		LOG_E("Init mutex failed");
		goto err_free_motor;
	}
	p_motor->period = 1000;
	p_motor->duty_cycle = 500;

	ret = export_pwm(PWM_CHIP, PWM_CHANNEL);
	if (!ret) {
		LOG_E("create motor failed");
		goto err_free_mutex;
	}

	pthread_mutex_lock(&p_motor->mutex);

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, p_motor->period);			// 设置周期(音调)
	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, p_motor->duty_cycle); // 设置占空比(强度)

	pthread_mutex_unlock(&p_motor->mutex);

	*p_priv = p_motor;
	motor = p_motor;

	return true;

err_free_mutex:
	pthread_mutex_destroy(&motor->mutex);

err_free_motor:
	free(motor);

	return false;
}

/**
 * @brief 开启/关闭
 * 
 * @param enable 
 */
void motor_control(bool enable)
{
	pthread_mutex_lock(&motor->mutex);

	if (!motor)
		return;

	enable_pwm(PWM_CHIP, PWM_CHANNEL, enable);

	pthread_mutex_unlock(&motor->mutex);
}

/**
 * @brief 提高占空比
 * 
 */
void app_motor_add_duty(size_t duty)
{
	if (!motor)
		return;

	pthread_mutex_lock(&motor->mutex);

	if (MAX_MOTOR_DUTY - duty < motor->duty_cycle)
		motor->duty_cycle = MAX_MOTOR_DUTY;
	else
		motor->duty_cycle += duty;

	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, motor->duty_cycle);

	pthread_mutex_unlock(&motor->mutex);
}

/**
 * @brief 减少占空比
 * 
 */
void app_motor_sub_duty(size_t duty)
{
	if (!motor)
		return;

	pthread_mutex_lock(&motor->mutex);

	if (motor->duty_cycle < duty)
		motor->duty_cycle = 0;
	else
		motor->duty_cycle -= duty;

	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, motor->duty_cycle);

	pthread_mutex_unlock(&motor->mutex);
}

/**
 * @brief 提高周期
 * 
 */
void app_motor_add_period(size_t period)
{
	if (!motor)
		return;

	pthread_mutex_lock(&motor->mutex);

	if (MAX_MOTOR_DUTY - period < motor->period)
		motor->period = MAX_MOTOR_PERIOD;
	else
		motor->period += period;

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, motor->period);

	pthread_mutex_unlock(&motor->mutex);
}

/**
 * @brief 减少周期
 * 
 */
void app_motor_sub_period(size_t period)
{
	if (!motor)
		return;

	pthread_mutex_lock(&motor->mutex);

	if (motor->period < period)
		motor->period = 0;
	else
		motor->period -= period;

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, motor->period);

	pthread_mutex_unlock(&motor->mutex);
}
