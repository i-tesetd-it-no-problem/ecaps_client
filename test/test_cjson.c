#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cjson/cJSON.h"

// 模拟发送 JSON 数据
void send_json_data(const char *json_string)
{
	printf("Transmitting JSON data: %s\n", json_string);
}

// 模拟接收 JSON 数据
const char *receive_json_data()
{
	// 这里模拟接收到的 JSON 数据，通常实际情况是从网络中读取
	return "{\"name\":\"Alice\",\"age\":30,\"is_student\":false,\"hobbies\":[\"reading\",\"coding\",\"hiking\"]}";
}

// 创建并发送 JSON 数据
void create_and_send_json()
{
	// 创建一个 JSON 对象
	cJSON *json = cJSON_CreateObject();

	// 添加一些键值对到 JSON 对象
	cJSON_AddStringToObject(json, "name", "Alice");
	cJSON_AddNumberToObject(json, "age", 30);
	cJSON_AddBoolToObject(json, "is_student", 0);

	// 创建一个嵌套的数组对象
	cJSON *hobbies = cJSON_CreateArray();
	cJSON_AddItemToArray(hobbies, cJSON_CreateString("reading"));
	cJSON_AddItemToArray(hobbies, cJSON_CreateString("coding"));
	cJSON_AddItemToArray(hobbies, cJSON_CreateString("hiking"));

	// 将数组添加到 JSON 对象中
	cJSON_AddItemToObject(json, "hobbies", hobbies);

	// 将 JSON 对象转换为紧凑的字符串
	char *json_string = cJSON_PrintUnformatted(json);
	if (json_string != NULL) {
		// 发送 JSON 字符串
		send_json_data(json_string);

		// 打印传输的 JSON 字符串（仅用于调试）
		printf("Sent JSON: %s\n", json_string);

		// 释放 JSON 字符串
		cJSON_free(json_string);
	}

	// 释放 JSON 对象
	cJSON_Delete(json);
}

// 接收并解析 JSON 数据
void receive_and_parse_json()
{
	// 模拟接收 JSON 数据
	const char *json_string = receive_json_data();

	// 解析 JSON 字符串
	cJSON *json = cJSON_Parse(json_string);
	if (json == NULL) {
		printf("Error parsing JSON data\n");
		return;
	}

	// 提取字符串字段
	cJSON *name = cJSON_GetObjectItem(json, "name");
	if (cJSON_IsString(name) && (name->valuestring != NULL)) {
		printf("Name: %s\n", name->valuestring);
	}

	// 提取数值字段
	cJSON *age = cJSON_GetObjectItem(json, "age");
	if (cJSON_IsNumber(age)) {
		printf("Age: %d\n", age->valueint);
	}

	// 提取布尔字段
	cJSON *is_student = cJSON_GetObjectItem(json, "is_student");
	if (cJSON_IsBool(is_student)) {
		printf("Is Student: %s\n", cJSON_IsTrue(is_student) ? "true" : "false");
	}

	// 提取数组字段
	cJSON *hobbies = cJSON_GetObjectItem(json, "hobbies");
	if (cJSON_IsArray(hobbies)) {
		printf("Hobbies:\n");
		cJSON *hobby = NULL;
		cJSON_ArrayForEach(hobby, hobbies)
		{
			if (cJSON_IsString(hobby)) {
				printf("  - %s\n", hobby->valuestring);
			}
		}
	}

	// 释放 JSON 对象
	cJSON_Delete(json);
}

int main()
{
	// 创建并发送 JSON 数据
	create_and_send_json();

	// 接收并解析 JSON 数据
	receive_and_parse_json();

	return 0;
}
