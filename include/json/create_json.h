#ifndef __CREATE_JSON_H__
#define __CREATE_JSON_H__

#include "json/sensor.h"

/**
 * @brief 获取传感器数据结构体指针(单例模式)
 * 
 * @return struct sensor_data* 
 */
struct sensor_data *get_sensor_data(void);

/**
 * @brief 获取当前传感器数据转换的Json字符串 (遵循 Docs/third_part/cJson/JsonSchema.json 规范)
 * 
 * @return char* 成功返回Json字符串，失败返回NULL
 */
char* get_cur_sensor_json(void);

#endif /* __CREATE_JSON_H__ */