/**
 * @file sensor.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 传感器数据定义
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

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdbool.h>

// 电压电流传感器
struct lmv358_sensor {
	double current; // 电流
	double voltage; // 电压
};

// 环境光/接近/红外传感器
struct ap3216c_sensor {
	double illuminance; // 光照强度
	double proximity;	// 接近度
	double infrared;	// 红外
};

// 温湿度传感器
struct si7006_sensor {
	double temperature; // 温度
	double humidity;	// 湿度
};

// 人体红外
struct rda226_sensor {
	bool detected; // 检测到人体
};

// 光闸/火焰传感器
struct itr9608_sensor {
	bool light_detected; // 光电开关检测状态
	bool flame_detected; // 火焰检测状态
};

// 心率/血氧传感器
struct max30102_sensor {
	double heart_rate;	 // 心率
	double blood_oxygen; // 血氧
};

// 所有传感器数据
struct sensor_data {
	struct lmv358_sensor lmv358;	 // 电压电流传感器
	struct ap3216c_sensor ap3216c;	 // 环境光/接近/红外传感器
	struct si7006_sensor si7006;	 // 湿度传感器
	struct rda226_sensor rda226;	 // 人体红外传感器
	struct itr9608_sensor itr9608;	 // 光闸/火焰传感器
	struct max30102_sensor max30102; // 心率/血氧传感器
};

#endif /* __SENSOR_H__ */
