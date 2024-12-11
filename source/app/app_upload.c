/**
 * @file app_upload.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 服务端上传模块
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

#include <stdio.h>
#include <stdlib.h>
#include "app/app_upload.h"

#include "json/sensor_json.h"
#include "ssl/ssl_client.h"

#include "utils/logger.h"

#include "app/app_upload.h"

#ifdef BOARD_ENV
#define SERVER_URL "https://49.51.40.135:8001/" // 服务器URL

#else

#define SERVER_URL "127.0.0.1:8001/" // 服务器URL

#endif

#define GET_URL SERVER_URL "test"				   // GET 请求URL
#define POST_URL SERVER_URL "submit"			   // POST 请求URL
#define SENSOR_URL SERVER_URL "submit_sensor_data" // POST 请求URL

static void app_upload_sensor_data(ssl_handle sh)
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

/**
 * @brief SSL客户端初始化
 * 
 * @param p_priv 私有数据二级指针
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool app_upload_init(void **p_priv)
{
	ssl_handle *sh = malloc(sizeof(ssl_handle));
	if (!sh) {
		LOG_E("Malloc ssl_handle failed");
		return false;
	}

	int ret = ssl_client_init(sh); // 初始化客户端
	if (ret) {
		LOG_E("Init ssl client failed");
		goto err_free_sh;
	}

	ret = ssl_client_connect(*sh, SERVER_URL); // 建立连接
	if (ret) {
		LOG_E("Connect ssl failed");
		goto err_free_client;
	}

	*p_priv = sh;

	return true;

err_free_client:
	ssl_client_free(*sh);

err_free_sh:
	free(sh);

	return false;
}

/**
 * @brief SSL客户端去初始化
 * 
 * @param priv 私有数据指针
 */
void app_upload_deinit(void *priv)
{
	if (!priv)
		return;

	ssl_handle *sh = priv;

	ssl_client_close(*sh);
	ssl_client_free(*sh);

	free(sh);
}

/**
 * @brief SSL客户端任务
 * 
 * @param priv 私有数据指针
 */
void app_upload_task(void *priv)
{
	if (!priv)
		return;

	ssl_handle *sh = priv;

	app_upload_sensor_data(*sh);
}