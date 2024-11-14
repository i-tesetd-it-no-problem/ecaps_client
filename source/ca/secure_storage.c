/**
 * @file secure_storage.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 安全存储/读取模块 与OPTEE的交互
 * @version 1.0
 * @date 2024-11-11
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <tee_client_api.h>
#include "utilities/logger.h"

#include "ca/secure_storage.h"

#ifdef BOARD_ENV // 开发板环境

#define SECURE_STORAGE_CMD_CREATE 0
#define SECURE_STORAGE_CMD_OPEN 1
#define SECURE_STORAGE_CMD_RENAME 2
#define SECURE_STORAGE_CMD_SEEK 3
#define SECURE_STORAGE_CMD_WRITE 4
#define SECURE_STORAGE_CMD_READ 5
#define SECURE_STORAGE_CMD_CLOSE 6
#define SECURE_STORAGE_CMD_DELETE 7
#define SECURE_STORAGE_CMD_EXISTS 8
#define SECURE_STORAGE_CMD_GET_ALL 9

static const TEEC_UUID storage_uuid = { 0xef83682b,
					0x8a80,
					0x45e0,
					{ 0x99, 0x93, 0xae, 0x58, 0x3a, 0x38, 0x66, 0x28 } };

struct storage_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
} context;

static bool create_cert_storage(struct storage_ctx *ctx, const char *obj_id, size_t file_size)
{
	if (!ctx || !obj_id || !file_size)
		return false;

	TEEC_Result res;
	TEEC_Operation op;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));
	op.paramTypes =
		TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)obj_id;
	op.params[0].tmpref.size = strlen(obj_id) + 1;
	op.params[1].value.a = file_size;

	res = TEEC_InvokeCommand(&ctx->sess, SECURE_STORAGE_CMD_CREATE, &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("created persistant object failed with code 0x%x origin 0x%x", res,
		      err_origin);
		return false;
	}

	return true;
}

static bool check_cert_in_storage(struct storage_ctx *ctx, const char *obj_id)
{
	if (!ctx || !obj_id)
		return false;

	TEEC_Result res;
	TEEC_Operation op;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)obj_id;
	op.params[0].tmpref.size = strlen(obj_id) + 1;

	res = TEEC_InvokeCommand(&ctx->sess, SECURE_STORAGE_CMD_EXISTS, &op, &err_origin);
	if (res == TEEC_ERROR_ITEM_NOT_FOUND) {
		LOG_I("%s is not exist in OPTEE\n", obj_id);
		return false;
	} else if (res != TEEC_SUCCESS) {
		LOG_E("check persistant object failed with code 0x%x origin 0x%x", res, err_origin);
		return false;
	}

	return true;
}

static bool open_cert_storage(struct storage_ctx *ctx, const char *obj_id)
{
	if (!ctx || !obj_id)
		return false;

	TEEC_Result res;
	TEEC_Operation op;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)obj_id;
	op.params[0].tmpref.size = strlen(obj_id) + 1;

	res = TEEC_InvokeCommand(&ctx->sess, SECURE_STORAGE_CMD_OPEN, &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("open persistant object failed with code 0x%x origin 0x%x", res, err_origin);
		return false;
	}

	return true;
}

typedef enum { TEE_DATA_SEEK_SET = 0, TEE_DATA_SEEK_CUR = 1, TEE_DATA_SEEK_END = 2 } TEE_Whence;
static bool seek_cert_storage(struct storage_ctx *ctx, const char *obj_id, int32_t offset,
			      TEE_Whence whence)
{
	if (!ctx || !obj_id)
		return false;

	TEEC_Result res;
	TEEC_Operation op;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));
	op.paramTypes =
		TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)obj_id;
	op.params[0].tmpref.size = strlen(obj_id) + 1;
	op.params[1].value.a = (uint32_t)offset;
	op.params[1].value.b = whence;

	res = TEEC_InvokeCommand(&ctx->sess, SECURE_STORAGE_CMD_SEEK, &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("seek persistant object failed with code 0x%x origin 0x%x", res, err_origin);
		return false;
	}

	return true;
}

static bool write_cert_storage(struct storage_ctx *ctx, const char *obj_id, char *content,
			       size_t size)
{
	if (!ctx || !obj_id || !content || !size)
		return false;

	TEEC_Result res;
	TEEC_Operation op;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
					 TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)obj_id;
	op.params[0].tmpref.size = strlen(obj_id) + 1;
	op.params[1].tmpref.buffer = content;
	op.params[1].tmpref.size = size;

	res = TEEC_InvokeCommand(&ctx->sess, SECURE_STORAGE_CMD_WRITE, &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("write persistant object failed with code 0x%x origin 0x%x", res, err_origin);
		return false;
	}

	return true;
}

static bool read_cert_storage(struct storage_ctx *ctx, const char *obj_id, char *content,
			      size_t size)
{
	if (!ctx || !obj_id || !content || !size)
		return false;

	TEEC_Result res;
	TEEC_Operation op;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
					 TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)obj_id;
	op.params[0].tmpref.size = strlen(obj_id) + 1;
	op.params[1].tmpref.buffer = content;
	op.params[1].tmpref.size = size;

	res = TEEC_InvokeCommand(&ctx->sess, SECURE_STORAGE_CMD_READ, &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("read persistant object failed with code 0x%x origin 0x%x", res, err_origin);
		return false;
	}

	return true;
}

static bool close_cert_storage(struct storage_ctx *ctx)
{
	if (!ctx)
		return false;

	TEEC_Result res;
	uint32_t err_origin;
	TEEC_Operation op;

	memset(&op, 0, sizeof(op));

	res = TEEC_InvokeCommand(&ctx->sess, SECURE_STORAGE_CMD_CLOSE, &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("close failed with code 0x%x origin 0x%x", res, err_origin);
		return false;
	}
	return true;
}

static bool prepare_tee_session(struct storage_ctx *ctx)
{
	uint32_t origin;
	TEEC_Result res;

	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS) {
		LOG_E("TEEC_InitializeContext failed with code 0x%x", res);
		return false;
	}

	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &storage_uuid, TEEC_LOGIN_PUBLIC, NULL, NULL,
			       &origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("TEEC_Opensession failed with code 0x%x origin 0x%x", res, origin);
		return false;
	}

	return true;
}

static void terminate_tee_session(struct storage_ctx *ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

/*********************************API*********************************/

/**
 * @brief find certification and save ti to TEE then delete it from file system
 * 
 * @param object_id TEE Object ID
 * @param local_file_path file path in local file system
 * @return true 
 * @return false 
 */
bool storage_and_delete(const char *object_id, const char *local_file_path)
{
	bool res;

	// 初始化TEE通信
	res = prepare_tee_session(&context);
	if (!res)
		return res;

	// 判断 TEE 中 存储对象是否存在
	res = check_cert_in_storage(&context, object_id);
	if (res)
		goto err_free_ctx;

	// 判断本地文件是否存在
	int fd = open(local_file_path, O_RDONLY);
	if (fd < 0) {
		LOG_E("open %s failed\n", local_file_path);
		goto err_free_ctx;
	}

	// 获取本地文件大小
	off_t file_size = lseek(fd, 0, SEEK_END);
	if (file_size == -1) {
		LOG_E("can't get %s size\n", local_file_path);
		goto err_close_fd;
	}
	lseek(fd, 0, SEEK_SET);

	// 动态申请内存存储文件内容
	char *content = (char *)malloc(file_size + 1);
	if (!content) {
		LOG_E("malloc %s memery failed\n", local_file_path);
		goto err_close_fd;
	}

	// 读取本地文件
	ssize_t read_bytes = read(fd, content, file_size);
	if (read_bytes == -1) {
		LOG_E("read %s failed\n", local_file_path);
		goto err_free;
	}
	content[read_bytes++] = '\0';

	// TEE中 创建 持久化存储对象
	res = create_cert_storage(&context, object_id, file_size);
	if (!res)
		goto err_free;

	// 打开对象
	res = open_cert_storage(&context, object_id);
	if (!res)
		goto err_free;

	// 写入对象
	res = write_cert_storage(&context, object_id, content, read_bytes);
	if (!res)
		goto err_free;

	// 释放资源
	terminate_tee_session(&context);
	close(fd);

	LOG_I("save %s to tee success\n", CLIENT_CER_PATH);

	if (remove(CLIENT_CER_PATH))
		LOG_W("delete %s filed!!!\n", CLIENT_CER_PATH);

	LOG_I("delete %s success\n", CLIENT_CER_PATH);
	return true;

err_free:
	free(content);
	content = NULL;

err_close_fd:
	close(fd);

err_free_ctx:
	terminate_tee_session(&context);

	return false;
}

/**
 * @brief read object
 * 
 * @param object_id TEE Object ID
 * @param buf read buffer
 * @param size buffer size
 * @return true 
 * @return false 
 */
bool read_tee_object(const char *object_id,unsigned char *buf, size_t *size)
{
	if (!buf || !size)
		return false;

	bool b_res;
	TEEC_Result res;
	TEEC_Operation op;
	uint32_t err_origin;

	// 初始化TEE通信
	b_res = prepare_tee_session(&context);
	if (!b_res)
		return b_res;

	// 指令 SECURE_STORAGE_CMD_GET_ALL 读取整个证书
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE,
					 TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)object_id;
	op.params[0].tmpref.size = strlen(object_id) + 1;
	op.params[1].tmpref.buffer = (void *)buf;
	op.params[1].tmpref.size = *size;

	res = TEEC_InvokeCommand(&context.sess, SECURE_STORAGE_CMD_GET_ALL, &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("read persistant object failed with code 0x%x origin 0x%x", res, err_origin);
		goto err_free_ctx;
	}

	*size = op.params[1].tmpref.size;

	terminate_tee_session(&context);

	return true;

err_free_ctx:
	terminate_tee_session(&context);

	return false;
}

#endif