/**
 * @file queue.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 循环队列组件,线程安全
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

#include <string.h>
#include <stdlib.h>
#include "utils/queue.h"

static size_t Q_MIN(size_t a, size_t b)
{
	return (a <= b) ? a : b;
}

bool queue_init(struct queue_info *q, size_t unit_bytes, uint8_t *buf, size_t count)
{
	if (!q || !buf || count == 0)
		return false;

	q->unit_bytes = unit_bytes;
	q->buf = buf;
	q->buf_size = count;
	q->rd = q->wr = 0;

	if (pthread_mutex_init(&q->mutex, NULL) != 0)
		return false;

	return true;
}

void queue_destroy(struct queue_info *q)
{
	if (!q)
		return;

	pthread_mutex_destroy(&q->mutex);
	/* 注意：缓冲区由外部管理,这里不负责释放 */
}

void queue_reset(struct queue_info *q)
{
	if (!q)
		return;

	pthread_mutex_lock(&q->mutex);
	q->rd = q->wr = 0;
	pthread_mutex_unlock(&q->mutex);
}

size_t queue_add(struct queue_info *q, const uint8_t *data, size_t units)
{
	if (!q || !data || units == 0)
		return 0;

	pthread_mutex_lock(&q->mutex);

	size_t spaces = q->buf_size - (q->wr - q->rd);
	if (spaces == 0) {
		pthread_mutex_unlock(&q->mutex);
		return 0;
	}

	size_t to_add = Q_MIN(units, spaces);
	size_t index = q->wr % q->buf_size;
	size_t tail_cnt = Q_MIN(to_add, q->buf_size - index);

	memcpy(q->buf + (index * q->unit_bytes), data, tail_cnt * q->unit_bytes);
	if (to_add > tail_cnt)
		memcpy(q->buf, data + (tail_cnt * q->unit_bytes), (to_add - tail_cnt) * q->unit_bytes);

	q->wr += to_add;

	pthread_mutex_unlock(&q->mutex);
	return to_add;
}

/* 从队列中获取数据 */
size_t queue_get(struct queue_info *q, uint8_t *data, size_t units)
{
	if (!q || !data || units == 0)
		return 0;

	pthread_mutex_lock(&q->mutex);

	size_t used = q->wr - q->rd;
	if (used == 0) {
		pthread_mutex_unlock(&q->mutex);
		return 0;
	}

	size_t to_get = Q_MIN(units, used);
	size_t index = q->rd % q->buf_size;
	size_t tail_cnt = Q_MIN(to_get, q->buf_size - index);

	memcpy(data, q->buf + (index * q->unit_bytes), tail_cnt * q->unit_bytes);
	if (to_get > tail_cnt)
		memcpy(data + (tail_cnt * q->unit_bytes), q->buf, (to_get - tail_cnt) * q->unit_bytes);

	q->rd += to_get;

	pthread_mutex_unlock(&q->mutex);
	return to_get;
}

size_t queue_peek(const struct queue_info *q, uint8_t *data, size_t units)
{
	if (!q || !data || units == 0)
		return 0;

	struct queue_info *non_const_q = (struct queue_info *)q;

	pthread_mutex_lock(&non_const_q->mutex);

	size_t used = non_const_q->wr - non_const_q->rd;
	if (used == 0) {
		pthread_mutex_unlock(&non_const_q->mutex);
		return 0;
	}

	size_t to_peek = Q_MIN(units, used);
	size_t index = non_const_q->rd % non_const_q->buf_size;
	size_t tail_cnt = Q_MIN(to_peek, non_const_q->buf_size - index);

	memcpy(data, non_const_q->buf + (index * non_const_q->unit_bytes),
		tail_cnt * non_const_q->unit_bytes);
	if (to_peek > tail_cnt)
		memcpy(data + (tail_cnt * non_const_q->unit_bytes), non_const_q->buf,
			(to_peek - tail_cnt) * non_const_q->unit_bytes);

	pthread_mutex_unlock(&non_const_q->mutex);
	return to_peek;
}

bool is_queue_empty(const struct queue_info *q)
{
	if (!q)
		return true;

	struct queue_info *non_const_q = (struct queue_info *)q;

	pthread_mutex_lock(&non_const_q->mutex);
	bool empty = (non_const_q->wr - non_const_q->rd) == 0;
	pthread_mutex_unlock(&non_const_q->mutex);

	return empty;
}

bool is_queue_full(const struct queue_info *q)
{
	if (!q)
		return false;

	struct queue_info *non_const_q = (struct queue_info *)q;

	pthread_mutex_lock(&non_const_q->mutex);
	bool full = (non_const_q->wr - non_const_q->rd) >= non_const_q->buf_size;
	pthread_mutex_unlock(&non_const_q->mutex);

	return full;
}
