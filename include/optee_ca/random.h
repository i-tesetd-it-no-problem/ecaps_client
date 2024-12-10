/**
 * @file random.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 随机数生成器, 由OPTEE提供
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

#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief 随机数生成器
 * 
 * @param p_rng 无用参数,为了适配mbedtls的接口, 可为NULL
 * @param buf 随机数缓冲区
 * @param len 随机数长度
 * @return int 成功返回0, 失败返回非0
 */
int random_generate(void *p_rng, unsigned char *buf, size_t len);

#endif