#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cjson/cJSON.h"
#include "unity.h"

// 模拟发送 JSON 数据
void send_json_data(const char *json_string) {
    printf("Transmitting JSON data: %s\n", json_string);
}

// 模拟接收 JSON 数据
const char *receive_json_data() {
    return "{\"name\":\"Alice\",\"age\":30,\"is_student\":false,\"hobbies\":[\"reading\",\"coding\",\"hiking\"]}";
}

// 创建 JSON 数据
char *create_json() {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "name", "Alice");
    cJSON_AddNumberToObject(json, "age", 30);
    cJSON_AddBoolToObject(json, "is_student", 0);

    cJSON *hobbies = cJSON_CreateArray();
    cJSON_AddItemToArray(hobbies, cJSON_CreateString("reading"));
    cJSON_AddItemToArray(hobbies, cJSON_CreateString("coding"));
    cJSON_AddItemToArray(hobbies, cJSON_CreateString("hiking"));

    cJSON_AddItemToObject(json, "hobbies", hobbies);

    char *json_string = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    return json_string;
}

// 解析 JSON 数据
int parse_json(const char *json_string, char *output, size_t output_size) {
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        return -1; // JSON 解析失败
    }

    snprintf(output, output_size, "Name: %s\nAge: %d\nIs Student: %s",
             cJSON_GetObjectItem(json, "name")->valuestring,
             cJSON_GetObjectItem(json, "age")->valueint,
             cJSON_IsTrue(cJSON_GetObjectItem(json, "is_student")) ? "true" : "false");

    cJSON_Delete(json);
    return 0;
}

void test_create_json() {
    char *json = create_json();
    TEST_ASSERT_NOT_NULL(json);
    TEST_ASSERT_TRUE(strstr(json, "\"name\":\"Alice\"") != NULL);
    TEST_ASSERT_TRUE(strstr(json, "\"age\":30") != NULL);
    TEST_ASSERT_TRUE(strstr(json, "\"is_student\":false") != NULL);
    free(json);
}

void test_parse_json() {
    const char *json_data = "{\"name\":\"Alice\",\"age\":30,\"is_student\":false}";
    char output[256] = {0};
    int result = parse_json(json_data, output, sizeof(output));
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(strstr(output, "Name: Alice") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "Age: 30") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "Is Student: false") != NULL);
}

void setUp(void)
{

}

void tearDown(void)
{

}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_create_json);
    RUN_TEST(test_parse_json);
    return UNITY_END();
}
