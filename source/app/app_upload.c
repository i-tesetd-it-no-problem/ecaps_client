#include "app/app_upload.h"
#include "app/algorithm.h"
#include "json/sensor_json.h"
#include <stdio.h>
#include "app/app_beep.h"
#include "app/app_digital.h"
#include "app/app_fans.h"
#include "app/app_led.h"
#include "app/app_si7006.h"
#include "app/app_motor.h"
#include "app/app_vol_cur.h"
#include "app/app_ap3216c.h"
#include "app/app_max30102.h"
#include "app/app_upload.h"

#include "ssl/ssl_client.h"
#include "utils/logger.h"

static ssl_handle sh = NULL;

#ifdef BOARD_ENV
#define SERVER_URL "https://49.51.40.135:8001/" // 服务器URL

#else

#define SERVER_URL "127.0.0.1:8001/" // 服务器URL

#endif

#define GET_URL SERVER_URL "test"				   // GET 请求URL
#define POST_URL SERVER_URL "submit"			   // POST 请求URL
#define SENSOR_URL SERVER_URL "submit_sensor_data" // POST 请求URL

bool app_upload_init(void)
{
	int ret = ssl_client_init(&sh); // 初始化客户端
	if (ret)
		return false;

	ret = ssl_client_connect(sh, SERVER_URL); // 建立连接
	if (ret) {
		ssl_client_free(sh);
		return false;
	}

	return true;
}

void app_upload_deinit(void)
{
	ssl_client_close(sh);
	ssl_client_free(sh);
}

static void app_upload_sensor_data(void)
{
	if (!sh)
		return;

	int ret;
	unsigned char resp_body[512] = { 0 };
	size_t recv_len;

	ret = ssl_client_post(
		sh, SENSOR_URL, get_cur_sensor_json(), resp_body, sizeof(resp_body), &recv_len);
	if (ret) {
		LOG_E("post faield\n");
		return;
	}

	printf("%s\n", resp_body);
}

void app_upload_task(void)
{
	app_upload_sensor_data();
}