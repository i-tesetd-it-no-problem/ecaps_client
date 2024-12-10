/**
 * @file queue.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 循环队列组件
 * @version 1.0
 * @date 2024-08-12
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#ifndef _QUEUE_H
#define _QUEUE_H

#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct queue_info {
    uint8_t *buf;          /* 缓冲区 */
    size_t unit_bytes;     /* 单元大小(字节数) */
    size_t buf_size;       /* 缓冲区容量(单位数) */
    size_t rd;             /* 读索引 */
    size_t wr;             /* 写索引 */
    pthread_mutex_t mutex; /* 互斥锁，用于线程安全 */
};

/**
 * @brief 初始化队列
 * 
 * @param q          指向队列实例的指针(由用户分配内存)
 * @param unit_bytes 每个单元的字节数
 * @param buf        指向预分配缓冲区的指针
 * @param count      缓冲区容量(单位数)
 * @return true 成功，false 失败
 */
bool queue_init(struct queue_info *q, size_t unit_bytes, uint8_t *buf, size_t count);

/**
 * @brief 销毁队列并释放资源
 * 
 * @param q 指向队列实例的指针
 */
void queue_destroy(struct queue_info *q);

/**
 * @brief 重置清空队列
 * 
 * @param q 指向队列实例的指针
 */
void queue_reset(struct queue_info *q);

/**
 * @brief 向队列中添加数据
 * 
 * @param q     指向队列实例的指针
 * @param data  指向要添加的数据的指针
 * @param units 要添加的单元数
 * @return size_t 成功添加的单元数
 */
size_t queue_add(struct queue_info *q, const uint8_t *data, size_t units);

/**
 * @brief 从队列中获取数据
 * 
 * @param q     指向队列实例的指针
 * @param data  指向存储获取数据的缓冲区的指针
 * @param units 要获取的单元数
 * @return size_t 成功获取的单元数
 */
size_t queue_get(struct queue_info *q, uint8_t *data, size_t units);

/**
 * @brief 查看队列中的数据，但不移除
 * 
 * @param q     指向队列实例的指针
 * @param data  指向存储查看数据的缓冲区的指针
 * @param units 要查看的单元数
 * @return size_t 成功查看的单元数
 */
size_t queue_peek(const struct queue_info *q, uint8_t *data, size_t units);

/**
 * @brief 判断队列是否为空
 * 
 * @param q 指向队列实例的指针
 * @return true 如果队列为空，false 否则
 */
bool is_queue_empty(const struct queue_info *q);

/**
 * @brief 判断队列是否已满
 * 
 * @param q 指向队列实例的指针
 * @return true 如果队列已满，false 否则
 */
bool is_queue_full(const struct queue_info *q);

#endif /* _QUEUE_H */
