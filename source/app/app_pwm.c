/**
 * @file app_pwm.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief PWM通用接口
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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "app/app_pwm.h"
#include "utils/logger.h"

#define PWM_CHIP_PATH "/sys/class/pwm/pwmchip%d"
#define PWM_PATH_FORMAT PWM_CHIP_PATH "/pwm%d"
#define PWM_EXPORT_PATH_FORMAT PWM_CHIP_PATH "/export"

/**
 * @brief 辅助函数：写入文件
 * 
 * @param path 文件路径
 * @param value 写入的值
 * @return bool 
 */
static bool write_to_file(const char *path, const char *value)
{
	int fd = open(path, O_WRONLY);
	if (fd < 0) {
		LOG_E("Failed to open file %s", path);
		return false;
	}

	size_t len = strlen(value);
	if (write(fd, value, len) != len) {
		LOG_E("Failed to write to file %s", path);
		close(fd);
		return false;
	}

	close(fd);
	return true;
}

/**
 * @brief 导出PWM
 * 
 * @param pwm_chip PWM编号
 * @param pwm_num PWM通道号
 * @return bool
 */
bool export_pwm(uint8_t pwm_chip, uint8_t pwm_num)
{
	char path[64] = { 0 };
	snprintf(path, sizeof(path), PWM_PATH_FORMAT, pwm_chip, pwm_num);
	struct stat st;
	if (stat(path, &st) == 0) {
		LOG_W("PWM %d already exported", pwm_num);
		return true;
	}

	snprintf(path, sizeof(path), PWM_EXPORT_PATH_FORMAT, pwm_chip);
	char buffer[4] = { 0 };
	snprintf(buffer, sizeof(buffer), "%d", pwm_num);
	if (!write_to_file(path, buffer)) {
		LOG_E("Failed to export PWM %d", pwm_num);
		return false;
	}

	return true;
}

/**
 * @brief 设置PWM周期
 * 
 * @param pwm_chip PWM编号
 * @param pwm_num PWM通道号
 * @param period_ns 纳秒
 * @return bool 
 */
bool set_pwm_period(uint8_t pwm_chip, uint8_t pwm_num, size_t period_ns)
{
	char path[64] = { 0 };
	snprintf(path, sizeof(path), PWM_PATH_FORMAT "/period", pwm_chip, pwm_num);
	char buffer[32] = { 0 };
	snprintf(buffer, sizeof(buffer), "%zu", period_ns);
	if (!write_to_file(path, buffer)) {
		LOG_E("Failed to set period for PWM %d", pwm_num);
		return false;
	}

	return true;
}

/**
 * @brief 设置PWM占空值
 * 
 * @param pwm_chip PWM编号
 * @param pwm_num PWM通道号
 * @param duty_ns 纳秒
 * @return bool 
 */
bool set_pwm_duty_cycle(uint8_t pwm_chip, uint8_t pwm_num, size_t duty_ns)
{
	char path[64] = { 0 };
	snprintf(path, sizeof(path), PWM_PATH_FORMAT "/duty_cycle", pwm_chip, pwm_num);
	char buffer[32] = { 0 };
	snprintf(buffer, sizeof(buffer), "%zu", duty_ns);
	if (!write_to_file(path, buffer)) {
		LOG_E("Failed to set duty cycle for PWM %d", pwm_num);
		return false;
	}

	return true;
}

/**
 * @brief 启用或禁用PWM
 * 
 * @param pwm_chip PWM编号
 * @param pwm_num PWM通道号
 * @param enable 开启(true)或关闭(false)
 * @return bool 
 */
bool enable_pwm(uint8_t pwm_chip, uint8_t pwm_num, bool enable)
{
	char path[64] = { 0 };
	snprintf(path, sizeof(path), PWM_PATH_FORMAT "/enable", pwm_chip, pwm_num);
	const char *value = enable ? "1" : "0";
	if (!write_to_file(path, value)) {
		LOG_E("Failed to %s PWM %d", enable ? "enable" : "disable", pwm_num);
		return false;
	}

	return true;
}
