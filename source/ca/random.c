
/**
 * @file random.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 随机数生成器, 由OPTEE提供
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

#include <string.h>
#include <ca/random.h>
#include <tee_client_api.h>
#include "utilities/logger.h"

#ifdef BOARD_ENV

#define TA_RNG_UUID                                                                                \
	{                                                                                          \
		0x5adc202b, 0x4e1b, 0x4590,                                                        \
		{                                                                                  \
			0x81, 0x93, 0x45, 0xf8, 0xff, 0x44, 0x23, 0x6e                             \
		}                                                                                  \
	}

#define GENERATE_RANDOM 0

struct rng_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

static bool prepare_tee_session(struct rng_ctx *ctx)
{
	TEEC_UUID uuid = TA_RNG_UUID;
	uint32_t origin;
	TEEC_Result res;

	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS) {
		LOG_E("TEEC_InitializeContext failed with code 0x%x", res);
		return false;
	}

	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL,
			       &origin);
	if (res != TEEC_SUCCESS) {
		LOG_E("TEEC_Opensession failed with code 0x%x origin 0x%x", res, origin);
		return false;
	}

	return true;
}

static void terminate_tee_session(struct rng_ctx *ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

int random_generate(void *p_rng, unsigned char *buf, size_t len)
{
	bool res;
	struct rng_ctx ctx;
	(void)p_rng;

	if (!buf || len == 0)
		return -1;

	res = prepare_tee_session(&ctx);
	if (!res)
		return -1;

	TEEC_Result ca_res;
	TEEC_Operation op;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = buf;
	op.params[0].tmpref.size = len;

	ca_res = TEEC_InvokeCommand(&ctx.sess, GENERATE_RANDOM, &op, &err_origin);
	if (ca_res != TEEC_SUCCESS) {
		LOG_E("TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
		return -1;
	}

	terminate_tee_session(&ctx);

	return 0;
}

#endif