#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "utils/logger.h"
#include "ssl/ssl_client.h"
#include "json/sensor_json.h"
#include "user_config.h"

static void test_secure_storage(void)
{
#ifdef BOARD_ENV
#include "optee_ca/secure_storage.h"
#define READ_BUFFER (1024 * 3)

	// delete_tee_object(CA_PEM_OBJ_ID);
	// delete_tee_object(CLIENT_CER_OBJ_ID);
	// delete_tee_object(CLIENT_KEY_OBJ_ID);

	// static size_t size = READ_BUFFER;
	// static unsigned char read_buf[READ_BUFFER] = { 0 };

	// 保存自建根证书到 OPTEE
	storage_and_delete(CA_PEM_OBJ_ID, BOARD_CA_PEM_PATH);
	// memset(read_buf, 0, READ_BUFFER);
	// size = READ_BUFFER;
	// read_tee_object(CA_PEM_OBJ_ID, read_buf, &size); // 读取证书
	// printf("%s", read_buf);

	// 保存客户端证书到 OPTEE
	storage_and_delete(CLIENT_CER_OBJ_ID, BOARD_CLIENT_CER_PATH);
	// memset(read_buf, 0, READ_BUFFER);
	// size = READ_BUFFER;
	// read_tee_object(CLIENT_CER_OBJ_ID, read_buf, &size); // 读取证书
	// printf("%s", read_buf);

	// 保存客户端私钥到 OPTEE
	storage_and_delete(CLIENT_KEY_OBJ_ID, BOARD_CLIENT_KEY_PATH);
	// memset(read_buf, 0, READ_BUFFER);
	// size = READ_BUFFER;
	// read_tee_object(CLIENT_KEY_OBJ_ID, read_buf, &size); // 读取证书
	// printf("%s", read_buf);
	// printf("\n");
#else

#endif
}

static void led_spark(void)
{
#ifdef BOARD_ENV
#define LED1_DEVICE "/dev/led1"

	int fd;
	uint8_t led_status = 0;
	ssize_t wtite_nums;

	fd = open(LED1_DEVICE, O_RDWR);
	if (fd == -1) {
		LOG_E("Failed to open device");
		return;
	}

	while (1) {
		wtite_nums = write(fd, &led_status, sizeof(led_status));
		if (wtite_nums == -1) {
			LOG_E("Failed to write to device");
			break;
		}
		led_status ^= 1;
		sleep(1);
	}

	close(fd);
#else

#endif
}

static void test_ssl(void)
{
#define SERVER_ADDR "49.51.40.135" // 服务器 IP 地址
#define SERVER_PORT "8001" // 服务器端口号
#define SERVER_URL "https://" SERVER_ADDR ":" SERVER_PORT
#define GET_URL SERVER_URL "/test"
#define POST_URL SERVER_URL "/submit_sensor_data"

	ssl_handle client;
	int ret;
	unsigned char resp_body[512]; // 回复数据
	size_t resp_body_len; // 回复数据长度

	ret = ssl_client_init(&client); // 初始化客户端
	if (ret != 0 || client == NULL) {
		LOG_E("Failed to initialize SSL client");
		return;
	}

	ret = ssl_client_connect(client, SERVER_URL); // 连接服务器
	if (ret != 0) {
		LOG_E("Failed to connect to server");
		ssl_client_free(client);
		client = NULL;
		return;
	}

	memset(resp_body, 0, sizeof(resp_body));
	// Get 请求
	ret = ssl_client_get(client, GET_URL, resp_body, sizeof(resp_body), &resp_body_len);
	if (ret != 0) {
		LOG_E("Failed to get data from server");
		ssl_client_close(client);
		ssl_client_free(client);
		client = NULL;
		return;
	}
	LOG_I("%s", resp_body);

	memset(resp_body, 0, sizeof(resp_body));
	get_sensor_data()->lmv358.current = 10.0; // 模拟电流
	get_sensor_data()->lmv358.voltage = 20.0; // 模拟电压
	char *post_sensor_data = get_cur_sensor_json();
	if (!post_sensor_data)
		return;
	ret = ssl_client_post(client, POST_URL, post_sensor_data, resp_body, sizeof(resp_body),
			      &resp_body_len);
	if (ret != 0) {
		LOG_E("Failed to post data to server");
		ssl_client_close(client);
		ssl_client_free(client);
		client = NULL;
		return;
	}
	LOG_I("%s", resp_body);
	free(post_sensor_data);

	ssl_client_close(client); // 关闭连接
	ssl_client_free(client); // 释放客户端
	client = NULL;

#undef SERVER_ADDR
#undef SERVER_PORT
#undef SERVER_URL
#undef GET_URL
#undef POST_URL
}

int main(void)
{
	// test_secure_storage(); // 保存证书等文件到OPTEE并读取

	test_ssl(); // SSL 连接测试

	// led_spark(); // LED 闪烁

	return 0;
}

// 主机
// rm -rf build;cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_host.cmake -DHOST_BUILD=ON -DCMAKE_BUILD_TYPE=Debug
// rm -rf build;cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_host.cmake -DHOST_BUILD=ON -DCMAKE_BUILD_TYPE=Release

// 开发板
// rm -rf build;cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_board.cmake -DHOST_BUILD=OFF -DCMAKE_BUILD_TYPE=Debug
// rm -rf build;cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_board.cmake -DHOST_BUILD=OFF -DCMAKE_BUILD_TYPE=Release

// cmake --build build --target clean
// scp ./build/output/ECAPS wenshuyu@192.168.1.6:/home/wenshuyu/ecaps
// scp ./build/output/test_logger wenshuyu@192.168.1.6:/home/wenshuyu/ecaps