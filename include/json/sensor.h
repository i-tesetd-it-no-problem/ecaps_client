#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdbool.h>

// 电压电流传感器
struct lmv358_sensor {
    double current;         // 电流
    double voltage;         // 电压
};

// 环境光/接近/红外传感器
struct ap3216c_sensor {
    double illuminance;     // 光照强度
    double proximity;       // 接近度
    double infrared;        // 红外
};

// 温湿度传感器
struct si7006_sensor {
    double temperature;     // 温度
    double humidity;        // 湿度
};

// 人体红外
struct rda226_sensor {
    bool detected;        // 检测到人体
};

// 光闸/火焰传感器
struct itr9608_sensor {
    bool light_detected;   // 光电开关检测状态
    bool flame_detected;        // 火焰检测状态
};

// 心率/血氧传感器
struct max30102_sensor {
    double heart_rate;  // 心率
    double blood_oxygen; // 血氧
};

// 所有传感器数据
struct sensor_data {
    struct lmv358_sensor lmv358;    // 电压电流传感器
    struct ap3216c_sensor ap3216c;  // 环境光/接近/红外传感器
    struct si7006_sensor si7006;    // 湿度传感器
    struct rda226_sensor rda226;    // 人体红外传感器
    struct itr9608_sensor itr9608;  // 光闸/火焰传感器
    struct max30102_sensor max30102;// 心率/血氧传感器
};

#endif /* __SENSOR_H__ */
