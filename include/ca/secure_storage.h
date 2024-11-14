/**
 * @file secure_storage.h
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
#ifndef _SECURE_STORAGE_H_
#define _SECURE_STORAGE_H_

#include <stdbool.h>
#include <stddef.h>

// scp ./tools/certification/client.crt wenshuyu@192.168.1.6:/home/wenshuyu/
#define CLIENT_CER_OBJ_ID   "client_crt"
#define CLIENT_CER_PATH     "/home/wenshuyu/client.crt"

// scp ./tools/certification/ca.pem wenshuyu@192.168.1.6:/home/wenshuyu/
#define ROOT_CA_OBJ_ID      "root_ca"
#define ROOT_CA_PATH        "/home/wenshuyu/ca.pem"

// scp ./tools/certification/client.key wenshuyu@192.168.1.6:/home/wenshuyu/
#define CLIENT_KEY_OBJ_ID   "priv_ley"
#define CLIENT_KEY_PATH     "/home/wenshuyu/client.key"

/**
 * @brief find certification and save ti to TEE then delete it from file system
 * 
 * @param object_id TEE Object ID
 * @param local_file_path file path in local file system
 * @return true 
 * @return false 
 */
bool storage_and_delete(const char* object_id, const char* local_file_path);

/**
 * @brief read object
 * 
 * @param object_id TEE Object ID
 * @param buf read buffer
 * @param size buffer size
 * @return true 
 * @return false 
 */
bool read_tee_object(const char* object_id,unsigned char *buf, size_t *size);

#endif /* _SECURE_STORAGE_H_ */