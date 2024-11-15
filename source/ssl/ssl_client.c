/**
 * @file ssl_client.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief SSL客户端接口
 * @version 1.0
 * @date 2024-11-13
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

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils/logger.h"
#include "ssl/build_request.h"
#include "ssl/https_parser.h"
#include "ssl/url_parser.h"
#include "ssl/ssl_client.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/error.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "user_config.h"
#include "optee_ca/random.h"
#include "optee_ca/secure_storage.h"

#define MBEDTLS_ERR_BUF_SIZE (512) // 错误信息缓冲区大小
char err_buf[MBEDTLS_ERR_BUF_SIZE]; // 错误信息缓冲区

pthread_mutex_t ssl_client_mutex = PTHREAD_MUTEX_INITIALIZER; // 互斥锁

#define OPTEE_OBJ_BUFFER (1024 * 3)

#ifdef PROJECT_ROOT
#define CA_PEM_PATH             PROJECT_ROOT "/" REL_CA_PEM_PATH
#define CLIENT_CRT_PATH         PROJECT_ROOT "/" REL_CLIENT_CRT_PATH
#define CLIENT_KEY_PATH         PROJECT_ROOT "/" REL_CLIENT_KEY_PATH
#else
#define CA_PEM_PATH             NULL
#define CLIENT_CRT_PATH         NULL
#define CLIENT_KEY_PATH         NULL
#endif


enum request_type {
	REQUEST_GET,
	REQUEST_POST,
	REQUEST_PUT,
	REQUEST_DELETE,
	REQUEST_HEAD,
	REQUEST_OPTIONS,
	REQUEST_PATCH,
	REQUEST_CONNECT,
	REQUEST_TRACE,
	REQUEST_UNKNOWN
};

// SSL客户端结构体
struct ssl_client_t {
	mbedtls_net_context net_ctx; // 网络上下文
	mbedtls_ssl_context ssl; // SSL上下文
	mbedtls_ssl_config conf; // SSL配置
	mbedtls_ctr_drbg_context ctr_drbg; // 随机数生成器上下文
	mbedtls_entropy_context entropy; // 熵源上下文
	mbedtls_x509_crt ca_cert; // CA证书
	mbedtls_x509_crt client_cert; // 客户端证书
	mbedtls_pk_context client_key; // 客户端私钥

	bool is_init; // 是否初始化
};

enum file_type {
	CA_PEM,         // CA证书
	CLIENT_CERT,    // 客户端证书
	CLIENT_KEY,     // 客户端私钥
};

// 获取证书密钥等
static bool get_pem_key(enum file_type type, unsigned char *buf, size_t *buf_len)
{
	if (!buf || !buf_len)
		return false;

	bool res;
	size_t out_size;

#ifdef BOARD_ENV
	out_size = *buf_len;
	char *obj;

	if (type == CA_PEM)
		obj = CA_PEM_OBJ_ID;
	else if (type == CLIENT_CERT)
		obj = CLIENT_CER_OBJ_ID;
	else if (type == CLIENT_KEY)
		obj = CLIENT_KEY_OBJ_ID;
	else
		return false;

	res = read_tee_object(obj, buf, &out_size);
	if (!res || (out_size + 1) > (*buf_len))
		return false;
	else {
		if (buf[out_size - 1] != '\0') {
			buf[out_size++] = '\0';
			*buf_len = out_size;
		}
		return true;
	}
#else
	char *file_path;

	if (type == CA_PEM)
		file_path = CA_PEM_PATH;
	else if (type == CLIENT_CERT)
		file_path = CLIENT_CRT_PATH;
	else if (type == CLIENT_KEY)
		file_path = CLIENT_KEY_PATH;
	else
		return false;

	// 打开文件
	int fd = open(file_path, O_RDONLY);
	if (fd < 0) {
		LOG_E("open %s failed\n", CA_PEM_PATH);
		return false;
	}

	// 获取文件大小
	off_t file_size = lseek(fd, 0, SEEK_END);
	if (file_size == -1) {
		LOG_E("can't get %s size\n", CA_PEM_PATH);
		goto err_close_fd;
	}
	lseek(fd, 0, SEEK_SET);

	// 缓冲区太小
	if ((file_size + 1) > (*buf_len)) {
		LOG_E("input buffer is too small\n");
		goto err_close_fd;
	}

	out_size = read(fd, buf, file_size);
	if (out_size < 0) {
		LOG_E("read %s failed\n", CA_PEM_PATH);
		goto err_close_fd;
	}

	if (buf[out_size - 1] != '\0') {
		buf[out_size++] = '\0';
		*buf_len = out_size;
	}
	return true;

err_close_fd:
	close(fd);

	return false;
#endif
}

static int ssl_client_send(ssl_handle client, const unsigned char *data, size_t len)
{
	if (!client || !client->is_init || !data || len == 0) {
		LOG_E("Invalid parameters for ssl_client_send\n");
		return -1;
	}

	size_t written = 0; // 已写入的字节数
	size_t max_payload =
		mbedtls_ssl_get_max_out_record_payload(&client->ssl); // 单条最大有效载荷
	int ret;
	char err_buf[MBEDTLS_ERR_BUF_SIZE];

	// 遍历全部写完
	while (written < len) {
		size_t chunk_size = (len - written > max_payload) ?
					    max_payload :
					    (len - written); // 当前待发送数据块大小
		const unsigned char *current_data = data + written; // 当前待发送起始位置
		size_t current_len = chunk_size;

		while (current_len > 0) {
			// 发送数据
			ret = mbedtls_ssl_write(&client->ssl, current_data, current_len);
			if (ret > 0) {
				written += ret;
				current_data += ret;
				current_len -= ret;
			} else {
				// 错误处理
				if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
				    ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
				    ret == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS ||
				    ret == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS ||
				    ret == MBEDTLS_ERR_SSL_RECEIVED_EARLY_DATA) {
					continue; // 继续
				} else if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
					// 对端通知关闭连接
					LOG_I("Peer closed connection gracefully during send.\n");
					mbedtls_ssl_session_reset(&client->ssl);
					return written; // 返回已写入的字节数
				} else if (ret == MBEDTLS_ERR_SSL_BAD_INPUT_DATA) {
					// 输入数据错误
					LOG_E("Bad input data error. Check max fragment length.\n");
					mbedtls_ssl_session_reset(&client->ssl);
					return ret;
				} else {
					// 其他错误
					memset(err_buf, 0, MBEDTLS_ERR_BUF_SIZE);
					mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
					LOG_E("Failed to send data: -0x%04X: %s\n", -ret, err_buf);
					mbedtls_ssl_session_reset(&client->ssl);
					return ret;
				}
			}
		}
	}
	return written; // 返回写入的总字节数
}

static int ssl_client_receive(ssl_handle client, unsigned char *buf, size_t buf_len)
{
	if (!client || !client->is_init || !buf || buf_len == 0) {
		LOG_E("Invalid parameters for ssl_client_receive\n");
		return -1;
	}

	int ret;
	char err_buf[MBEDTLS_ERR_BUF_SIZE];

	while (1) {
		// 接收数据
		ret = mbedtls_ssl_read(&client->ssl, buf, buf_len);
		if (ret > 0) {
			// 成功读取到数据
			return ret;
		} else if (ret == 0) {
			LOG_I("Connection closed by peer.\n");
			mbedtls_ssl_session_reset(&client->ssl); // 重置 SSL 会话
			return 0; // 对端关闭了连接
		} else {
			// 错误处理
			if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
			    ret == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS ||
			    ret == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS) {
				// 需要再次调用继续尝试读取
				continue;
			} else if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
				// 对端通知关闭连接
				LOG_I("Peer closed connection gracefully.\n");
				mbedtls_ssl_session_reset(&client->ssl);
				return ret;
			} else if (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) {
				// TLS1.3的规定
				LOG_D("Received new session ticket. Continuing.\n");
				continue;
			} else {
				// 其他错误
				memset(err_buf, 0, MBEDTLS_ERR_BUF_SIZE);
				mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
				LOG_E("Failed to read data: -0x%04X: %s\n", -ret, err_buf);
				mbedtls_ssl_session_reset(&client->ssl);
				return ret;
			}
		}
	}
}

static int ssl_client_fetch(ssl_handle client, enum request_type req_type, const char *url,
			    const char *content_type, const char *body,
			    unsigned char *response_body, size_t response_buffer_size,
			    size_t *response_body_len)
{
	if (!client || !client->is_init || !url || !response_body || !response_body_len) {
		LOG_E("Invalid parameters for ssl_client_fetch\n");
		return -1;
	}

	int ret;
	char request_str[1024] = { 0 }; // 构建请求字符串

	// 解析URL
	url_info_t *url_info = parse_url(url);
	if (!url_info) {
		LOG_E("Cannot parse URL: %s\n", url);
		return -1;
	}

	// 构建HTTP请求
	if (req_type == REQUEST_GET) {
		// GET请求
		ret = BUILD_GET(url, request_str, sizeof(request_str));
		if (ret != 0) {
			LOG_E("Build GET request failed\n");
			free_url_info(url_info);
			return -1;
		}
	} else if (req_type == REQUEST_POST) {
		// POST请求
		if (content_type && strcmp(content_type, "application/json") == 0) {
			// JSON Body
			ret = BUILD_POST_JSON(url, body, request_str, sizeof(request_str));
			if (ret != 0) {
				LOG_E("Build JSON-encoded POST request failed\n");
				free_url_info(url_info);
				return -1;
			}
		} else {
			// 空POST请求
			ret = BUILD_POST_EMPTY(url, request_str, sizeof(request_str)); // 空POST请求
			if (ret != 0) {
				LOG_E("Build empty POST request failed\n");
				free_url_info(url_info);
				return -1;
			}
		}
	} else {
		LOG_E("Unsupported HTTP method: %d\n", req_type);
		free_url_info(url_info);
		return -1;
	}

	LOG_D("Constructed request:\n%s\n", request_str);

	pthread_mutex_lock(&ssl_client_mutex); // 加锁

	// 发送请求
	ret = ssl_client_send(client, (const unsigned char *)request_str, strlen(request_str));
	if (ret <= 0) {
		LOG_E("Failed to send request: %d\n", ret);
		pthread_mutex_unlock(&ssl_client_mutex); // 解锁
		free_url_info(url_info);
		return -1;
	}

	// 初始化HTTP响应解析器
	https_resp_handle parser_handle = NULL;
	if (https_response_parser_init(&parser_handle)) {
		LOG_E("Failed to initialize HTTP response parser\n");
		pthread_mutex_unlock(&ssl_client_mutex); // 解锁
		free_url_info(url_info);
		return -1;
	}

	size_t total_received = 0; // 总共接收到的字节数

	while (total_received < response_buffer_size) {
		// 接收数据
		ret = ssl_client_receive(client, response_body + total_received,
					 response_buffer_size - total_received);
		if (ret < 0) {
			LOG_E("Failed to receive data: %d\n", ret);
			https_response_parser_free(&parser_handle);
			pthread_mutex_unlock(&ssl_client_mutex); // 解锁
			free_url_info(url_info);
			return -1;
		} else if (ret == 0) {
			LOG_I("Peer closed the connection.\n");
			break;
		}

		// 更新总接收字节数
		total_received += ret;

		// 解析响应
		ret = https_response_parser_update(parser_handle,
						   response_body + total_received - ret, ret);
		if (ret < 0) {
			LOG_E("Failed to parse HTTP response\n");
			https_response_parser_free(&parser_handle);
			pthread_mutex_unlock(&ssl_client_mutex); // 解锁
			free_url_info(url_info);
			return -1;
		}

		// 检查解析是否完成
		int is_complete = https_response_parser_is_complete(parser_handle);
		if (is_complete == 1) {
			// 完成
			const unsigned char *body_ptr; // 指向body的指针
			size_t body_len_local; // body 长度
			// 获取body
			ret = https_response_parser_get_body(parser_handle, response_body,
							     total_received, &body_ptr,
							     &body_len_local);
			if (ret == 0) {
				// 缓冲不足
				if (body_len_local > response_buffer_size - 1) {
					LOG_E("Provided response buffer is too small. Required: %zu, Provided: %zu\n",
					      body_len_local, response_buffer_size);
					https_response_parser_free(&parser_handle);
					pthread_mutex_unlock(&ssl_client_mutex); // 解锁
					free_url_info(url_info);
					return -1;
				}
				// 复制body到response_body
				memcpy(response_body, body_ptr, body_len_local);
				response_body[body_len_local] = '\0';
				*response_body_len = body_len_local;
				LOG_D("Received %zu bytes of response body.\n", body_len_local);
				// 释放资源
				https_response_parser_free(&parser_handle);
				pthread_mutex_unlock(&ssl_client_mutex); // 解锁
				free_url_info(url_info);
				return 0;
			} else {
				// 获取body失败
				LOG_E("Failed to get HTTP response body\n");
				https_response_parser_free(&parser_handle);
				pthread_mutex_unlock(&ssl_client_mutex); // 解锁
				free_url_info(url_info);
				return -1;
			}
		} else if (is_complete == -1) {
			// 错误
			LOG_E("Error occurred during HTTP parsing\n");
			https_response_parser_free(&parser_handle);
			pthread_mutex_unlock(&ssl_client_mutex); // 解锁
			free_url_info(url_info);
			return -1;
		}
		// 解析未完成，继续接收数据
	}

	//解析仍未完成
	LOG_E("Response parsing incomplete or buffer overflow\n");
	https_response_parser_free(&parser_handle);
	pthread_mutex_unlock(&ssl_client_mutex); // 解锁
	free_url_info(url_info);
	return -1;
}

int ssl_client_init(ssl_handle *client)
{
	if (!client) {
		LOG_E("ssl_client_init: clirnt pointer is null\n");
		return -1;
	}

	*client = malloc(sizeof(struct ssl_client_t));
	if (*client == NULL) {
		LOG_E("alloc ssl_client_t failed\n");
		return -1;
	}

	mbedtls_net_init(&(*client)->net_ctx);
	mbedtls_ssl_init(&(*client)->ssl);
	mbedtls_ssl_config_init(&(*client)->conf);
	mbedtls_ctr_drbg_init(&(*client)->ctr_drbg);
	mbedtls_entropy_init(&(*client)->entropy);
	mbedtls_x509_crt_init(&(*client)->ca_cert);
	mbedtls_x509_crt_init(&(*client)->client_cert);
	mbedtls_pk_init(&(*client)->client_key);
	(*client)->is_init = true;

	return 0;
}

void ssl_client_free(ssl_handle client)
{
	if (!client) {
		LOG_E("ssl_client_free: client handle is null\n");
		return;
	}

	if (!client->is_init) {
		LOG_E("ssl_client_free: client is not initialized\n");
		return;
	}

	// 释放各个资源
	mbedtls_net_free(&client->net_ctx);
	mbedtls_ssl_free(&client->ssl);
	mbedtls_ssl_config_free(&client->conf);
	mbedtls_ctr_drbg_free(&client->ctr_drbg);
	mbedtls_entropy_free(&client->entropy);
	mbedtls_x509_crt_free(&client->ca_cert);
	mbedtls_x509_crt_free(&client->client_cert);
	mbedtls_pk_free(&client->client_key);
	client->is_init = false;
	free(client);
}

int ssl_client_connect(ssl_handle client, const char *url)
{
	if (!client || !url) {
		LOG_E("ssl_client_connect: client is invalid\n");
		return -1;
	}

	int ret;
	bool rd_ret;
	;
	const char *pers = "ssl_client";
	unsigned char save_buf[OPTEE_OBJ_BUFFER] = { 0 };
	size_t save_len = OPTEE_OBJ_BUFFER;

	// 解析URL
	url_info_t *url_info = parse_url(url);
	if (!url_info) {
		LOG_E("Can't parse URL: %s\n", url);
		return -1;
	}

	// 设置随机数种子
	ret = mbedtls_ctr_drbg_seed(&client->ctr_drbg, mbedtls_entropy_func, &client->entropy,
				    (const unsigned char *)pers, strlen(pers));
	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Set seed failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

	// 初始化PSA Crypto库
	ret = psa_crypto_init();
	if (ret != PSA_SUCCESS) {
		LOG_E("Initialize PSA crypto failed: %d\n", ret);
		free_url_info(url_info);

		return -1;
	}

	// 加载CA根证书
    save_len = OPTEE_OBJ_BUFFER;
    memset(save_buf, 0, save_len);
	rd_ret = get_pem_key(CA_PEM, save_buf, &save_len);
	if (!rd_ret) {
		LOG_E("read ca certification faild\n");
		free_url_info(url_info);
		return -1;
	}
	ret = mbedtls_x509_crt_parse(&client->ca_cert, save_buf, save_len);
	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Parse CA root certificate failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

	// 加载客户端证书
    save_len = OPTEE_OBJ_BUFFER;
    memset(save_buf, 0, save_len);
	rd_ret = get_pem_key(CLIENT_CERT, save_buf, &save_len);
	if (!rd_ret) {
		LOG_E("read client certification faild\n");
		free_url_info(url_info);
		return -1;
	}
	ret = mbedtls_x509_crt_parse(&client->client_cert, save_buf, save_len);
	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Parse client certificate failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

	// 加载客户端私钥
    save_len = OPTEE_OBJ_BUFFER;
    memset(save_buf, 0, save_len);
	rd_ret = get_pem_key(CLIENT_KEY, save_buf, &save_len);
	if (!rd_ret) {
		LOG_E("read client certification faild\n");
		free_url_info(url_info);
		return -1;
	}
#ifdef BOARD_ENV
	ret = mbedtls_pk_parse_key(&client->client_key, save_buf, save_len, NULL, 0, random_generate,
				   NULL);
#else
	ret = mbedtls_pk_parse_key(&client->client_key, save_buf, save_len, NULL, 0,
				   mbedtls_ctr_drbg_random, &client->ctr_drbg);
#endif

	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Parse client private key failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

	// 设置SSL配置
	ret = mbedtls_ssl_config_defaults(&client->conf,
					  MBEDTLS_SSL_IS_CLIENT, // 客户端模式
					  MBEDTLS_SSL_TRANSPORT_STREAM, // TCP
					  MBEDTLS_SSL_PRESET_DEFAULT); // 默认配置
	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Set SSL config failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

	// TLS 1.3
	mbedtls_ssl_conf_min_version(&client->conf, MBEDTLS_SSL_MAJOR_VERSION_3,
				     MBEDTLS_SSL_MINOR_VERSION_4);
	mbedtls_ssl_conf_max_version(&client->conf, MBEDTLS_SSL_MAJOR_VERSION_3,
				     MBEDTLS_SSL_MINOR_VERSION_4);

	// 设置认证模式和随机数生成器
	mbedtls_ssl_conf_authmode(&client->conf, MBEDTLS_SSL_VERIFY_REQUIRED); // 需要认证服务端证书

#ifdef BOARD_ENV
	mbedtls_ssl_conf_rng(&client->conf, random_generate, NULL);
#else
	mbedtls_ssl_conf_rng(&client->conf, mbedtls_ctr_drbg_random,
			     &client->ctr_drbg); // 设置随机数生成回调
#endif
	mbedtls_ssl_conf_ca_chain(&client->conf, &client->ca_cert, NULL); // 设置证书认证参数

	// 设置客户端证书和私钥
	ret = mbedtls_ssl_conf_own_cert(&client->conf, &client->client_cert, &client->client_key);
	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Set client certificate failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

	// 设置SSL上下文
	ret = mbedtls_ssl_setup(&client->ssl, &client->conf);
	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Set SSL context failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

	// 设置服务器主机名（用于SNI和证书验证）
	ret = mbedtls_ssl_set_hostname(&client->ssl, url_info->host);
	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Set hostname failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

    // 设置SSL的I/O接口 可替换为自己的回调
	mbedtls_ssl_set_bio(&client->ssl, &client->net_ctx, mbedtls_net_send, mbedtls_net_recv,
			    NULL);

	// 连接到服务器
	LOG_D("Connecting to %s:%s...\n", url_info->host, url_info->port);
	ret = mbedtls_net_connect(&client->net_ctx, url_info->host, url_info->port,
				  MBEDTLS_NET_PROTO_TCP);
	if (ret != 0) {
		mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
		LOG_E("Connect failed: -0x%04X: %s\n", -ret, err_buf);
		free_url_info(url_info);

		return -1;
	}

	// 执行SSL握手
	LOG_D("Performing SSL handshake...\n");
	while ((ret = mbedtls_ssl_handshake(&client->ssl)) != 0) {
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			mbedtls_strerror(ret, err_buf, MBEDTLS_ERR_BUF_SIZE);
			LOG_E("SSL handshake failed: -0x%04X: %s\n", -ret, err_buf);
			mbedtls_ssl_session_reset(&client->ssl);
			mbedtls_net_free(&client->net_ctx);
			free_url_info(url_info);

			return -1;
		}
		// 如果返回 WANT_READ 或 WANT_WRITE，继续握手
	}

	// 验证服务器证书
	uint32_t flags = mbedtls_ssl_get_verify_result(&client->ssl);
	if (flags != 0) {
		char vrfy_buf[512];
		mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
		LOG_E("Validate server certificate failed:\n%s\n", vrfy_buf);
		mbedtls_ssl_session_reset(&client->ssl);
		mbedtls_net_free(&client->net_ctx);
		free_url_info(url_info);

		return -1;
	}

	LOG_D("SSL handshake completed.\n");

	free_url_info(url_info);

	LOG_D("Session saved.\n");
	return 0;
}

void ssl_client_close(ssl_handle client)
{
	if (!client) {
		LOG_E("ssl_client_close: client handle is unvalid\n");
		return;
	}

	mbedtls_ssl_close_notify(&client->ssl); // 通知对端关闭连接
	mbedtls_net_free(&client->net_ctx); // 释放网络资源
}

int ssl_client_get(ssl_handle client, const char *url, unsigned char *response_body,
		   size_t response_buffer_size, size_t *response_body_len)
{
	if (!client || !client->is_init || !url || !response_body || !response_body_len) {
		LOG_E("ssl_client_get: parameter is unvalid\n");
		return -1;
	}

	return ssl_client_fetch(client, REQUEST_GET, url, NULL, NULL, response_body,
				response_buffer_size, response_body_len);
}

int ssl_client_post(ssl_handle client, const char *url, const char *json_body,
		    unsigned char *response_body, size_t response_buffer_size,
		    size_t *response_body_len)
{
	if (!client || !client->is_init || !url || !response_body || !response_body_len) {
		LOG_E("ssl_client_post: parameter is unvalid\n");
		return -1;
	}

	return ssl_client_fetch(client, REQUEST_POST, url, "application/json", json_body,
				response_body, response_buffer_size, response_body_len);
}