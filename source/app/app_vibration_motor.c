/**
 * @file app_vibration_motor.c
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

#include "app/app_vibration_motor.h"
#include "app/app_pwm.h"
#include "utils/logger.h"
#include <stdbool.h>
#include <pthread.h>

#define PWM_CHIP (8U)	 // PWM 号
#define PWM_CHANNEL (0U) // PWM 通道 1
#define CALC_STEP (10)	 // 步长 10

struct app_vibration_motor {
	size_t period;		   // 周期（控制振动频率）
	size_t duty_cycle;	   // 占空比（控制振动强度）
	pthread_mutex_t mutex; // 互斥锁
	bool inited;		   // 初始化标志
};

// 实例化
struct app_vibration_motor vibration_motor = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.period = 1000,	   // 默认周期
	.duty_cycle = 500, // 默认占空值
	.inited = false,
};

/**
 * @brief 初始化震动马达
 * 
 * @return true 
 * @return false 
 */
bool app_vibration_motor_init(void)
{
	static bool inited = false;
	if (inited)
		return inited;
	inited = true;

	bool ret = false;

	ret = export_pwm(PWM_CHIP, PWM_CHANNEL);
	if (!ret) {
		LOG_E("create vibration motor failed");
		return ret;
	}

	pthread_mutex_lock(&vibration_motor.mutex);

	set_pwm_period(PWM_CHIP, PWM_CHANNEL, vibration_motor.period); // 设置周期（控制震动频率）
	set_pwm_duty_cycle(
		PWM_CHIP, PWM_CHANNEL, vibration_motor.duty_cycle); // 设置占空比（控制振动强度）

	pthread_mutex_unlock(&vibration_motor.mutex);

	vibration_motor.inited = true;

	return true;
}

/**
 * @brief 开启/关闭震动马达
 * 
 * @param enable 
 */
void vibration_motor_control(bool enable)
{
	pthread_mutex_lock(&vibration_motor.mutex);

	if (!vibration_motor.inited)
		return;

	enable_pwm(PWM_CHIP, PWM_CHANNEL, enable);

	pthread_mutex_unlock(&vibration_motor.mutex);
}

/**
 * @brief 提升震动强度（增加占空比）
 * 
 */
void app_vibration_motor_stronger(void)
{
	pthread_mutex_lock(&vibration_motor.mutex);

	if (!vibration_motor.inited)
		return;

	vibration_motor.duty_cycle += CALC_STEP;
	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, vibration_motor.duty_cycle);

	pthread_mutex_unlock(&vibration_motor.mutex);
}

/**
 * @brief 降低震动强度（减少占空比）
 * 
 */
void app_vibration_motor_weaker(void)
{
	pthread_mutex_lock(&vibration_motor.mutex);

	if (!vibration_motor.inited)
		return;

	vibration_motor.duty_cycle -= CALC_STEP;
	set_pwm_duty_cycle(PWM_CHIP, PWM_CHANNEL, vibration_motor.duty_cycle);

	pthread_mutex_unlock(&vibration_motor.mutex);
}

/**
 * @brief 提升震动频率（增加振动速度）
 * 
 */
void app_vibration_motor_faster(void)
{
	pthread_mutex_lock(&vibration_motor.mutex);

	if (!vibration_motor.inited)
		return;

	vibration_motor.period += 100;
	set_pwm_period(PWM_CHIP, PWM_CHANNEL, vibration_motor.period);

	pthread_mutex_unlock(&vibration_motor.mutex);
}

/**
 * @brief 降低震动频率（减慢振动速度）
 * 
 */
void app_vibration_motor_slower(void)
{
	pthread_mutex_lock(&vibration_motor.mutex);

	if (!vibration_motor.inited)
		return;

	vibration_motor.period -= 100;
	set_pwm_period(PWM_CHIP, PWM_CHANNEL, vibration_motor.period);

	pthread_mutex_unlock(&vibration_motor.mutex);
}
