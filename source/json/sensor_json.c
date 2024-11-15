#include "json/sensor_json.h"
#include "utils/logger.h"
#include "cjson/cJSON.h"
#include <time.h>

static struct sensor_data _s_d = {0};

struct sensor_data *get_sensor_data(void)
{
	return &_s_d;
}

char* get_cur_sensor_json(void)
{
	// 创建 JSON 根对象
	cJSON *root = cJSON_CreateObject();
	if (root == NULL) {
		LOG_E("Failed to create JSON root object");
		return NULL;
	}

	LOG_D("Starting to create JSON for sensor data");

	// 电压电流传感器数据
	cJSON *lmv358 = cJSON_CreateObject();
	if (lmv358 == NULL) {
		LOG_E("Failed to create 'lmv358' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(lmv358, "current", _s_d.lmv358.current)) {
		LOG_E("Failed to add 'current' to 'lmv358' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(lmv358, "voltage", _s_d.lmv358.voltage)) {
		LOG_E("Failed to add 'voltage' to 'lmv358' JSON object");
		goto end;
	}
	if (!cJSON_AddItemToObject(root, "lmv358", lmv358)) {
		LOG_E("Failed to add 'lmv358' to root JSON object");
		goto end;
	}
	LOG_D("Added 'lmv358' data to JSON");

	// 环境光/接近/红外传感器数据
	cJSON *ap3216c = cJSON_CreateObject();
	if (ap3216c == NULL) {
		LOG_E("Failed to create 'ap3216c' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(ap3216c, "illuminance", _s_d.ap3216c.illuminance)) {
		LOG_E("Failed to add 'illuminance' to 'ap3216c' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(ap3216c, "proximity", _s_d.ap3216c.proximity)) {
		LOG_E("Failed to add 'proximity' to 'ap3216c' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(ap3216c, "infrared", _s_d.ap3216c.infrared)) {
		LOG_E("Failed to add 'infrared' to 'ap3216c' JSON object");
		goto end;
	}
	if (!cJSON_AddItemToObject(root, "ap3216c", ap3216c)) {
		LOG_E("Failed to add 'ap3216c' to root JSON object");
		goto end;
	}
	LOG_D("Added 'ap3216c' data to JSON");

	// 温湿度传感器数据
	cJSON *si7006 = cJSON_CreateObject();
	if (si7006 == NULL) {
		LOG_E("Failed to create 'si7006' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(si7006, "temperature", _s_d.si7006.temperature)) {
		LOG_E("Failed to add 'temperature' to 'si7006' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(si7006, "humidity", _s_d.si7006.humidity)) {
		LOG_E("Failed to add 'humidity' to 'si7006' JSON object");
		goto end;
	}
	if (!cJSON_AddItemToObject(root, "si7006", si7006)) {
		LOG_E("Failed to add 'si7006' to root JSON object");
		goto end;
	}
	LOG_D("Added 'si7006' data to JSON");

	// 人体红外传感器数据
	cJSON *rda226 = cJSON_CreateObject();
	if (rda226 == NULL) {
		LOG_E("Failed to create 'rda226' JSON object");
		goto end;
	}
	if (!cJSON_AddBoolToObject(rda226, "detected", _s_d.rda226.detected)) {
		LOG_E("Failed to add 'detected' to 'rda226' JSON object");
		goto end;
	}
	if (!cJSON_AddItemToObject(root, "rda226", rda226)) {
		LOG_E("Failed to add 'rda226' to root JSON object");
		goto end;
	}
	LOG_D("Added 'rda226' data to JSON");

	// 光闸/火焰传感器数据
	cJSON *itr9608 = cJSON_CreateObject();
	if (itr9608 == NULL) {
		LOG_E("Failed to create 'itr9608' JSON object");
		goto end;
	}
	if (!cJSON_AddBoolToObject(itr9608, "light_detected", _s_d.itr9608.light_detected)) {
		LOG_E("Failed to add 'light_detected' to 'itr9608' JSON object");
		goto end;
	}
	if (!cJSON_AddBoolToObject(itr9608, "flame_detected", _s_d.itr9608.flame_detected)) {
		LOG_E("Failed to add 'flame_detected' to 'itr9608' JSON object");
		goto end;
	}
	if (!cJSON_AddItemToObject(root, "itr9608", itr9608)) {
		LOG_E("Failed to add 'itr9608' to root JSON object");
		goto end;
	}
	LOG_D("Added 'itr9608' data to JSON");

	// 心率/血氧传感器数据
	cJSON *max30102 = cJSON_CreateObject();
	if (max30102 == NULL) {
		LOG_E("Failed to create 'max30102' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(max30102, "heart_rate", _s_d.max30102.heart_rate)) {
		LOG_E("Failed to add 'heart_rate' to 'max30102' JSON object");
		goto end;
	}
	if (!cJSON_AddNumberToObject(max30102, "blood_oxygen", _s_d.max30102.blood_oxygen)) {
		LOG_E("Failed to add 'blood_oxygen' to 'max30102' JSON object");
		goto end;
	}
	if (!cJSON_AddItemToObject(root, "max30102", max30102)) {
		LOG_E("Failed to add 'max30102' to root JSON object");
		goto end;
	}
	LOG_D("Added 'max30102' data to JSON");

    // 添加时间戳
    time_t now = time(NULL);
    if (cJSON_AddNumberToObject(root, "timestamp", (double)now) == NULL) {
        LOG_E("Failed to add timestamp to JSON\n");
        goto end;
    }

	// 转换为 JSON 字符串
	char *json_string = cJSON_PrintUnformatted(root);
	if (json_string == NULL) {
		LOG_E("Failed to print JSON string");
		goto end;
	}
	LOG_D("Successfully created JSON string");

	// 清理 JSON 对象
	cJSON_Delete(root);
	return json_string;

end:
	// 释放内存
	cJSON_Delete(root);
	LOG_E("Failed to create JSON");
	return NULL;
}
