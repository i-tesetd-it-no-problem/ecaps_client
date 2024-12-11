/**
 * @file epoll_timer.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 定时器任务事件驱动组件
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

#ifndef _EPOLL_TIMER_H
#define _EPOLL_TIMER_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief epoll定时器系统句柄
 */
typedef struct epoll_timer *et_handle;

/**
 * @brief 周期任务初始化函数。成功时返回true，p_priv用于保存自定义指针
 */
typedef bool (*timer_task_init)(void **p_priv);

/**
 * @brief 周期任务处理函数，priv为初始化时保存的指针
 */
typedef void (*timer_task_entry)(void *priv);

/**
 * @brief 周期任务销毁函数，用于释放资源，priv为初始化时保存的指针
 */
typedef void (*timer_task_deinit)(void *priv);

/**
 * @brief 周期任务信息
 */
struct epoll_timer_task {
	char *task_name;			// 任务名
	timer_task_init f_init;		// 可为NULL
	timer_task_entry f_entry;	// 可为NULL
	timer_task_deinit f_deinit; // 可为NULL
	size_t period_ms;			// 任务周期(毫秒)
};

/**
 * @brief 创建epoll监听句柄
 * 
 * @return et_handle 失败返回NULL，成功返回句柄
 */
et_handle epoll_timer_create(void);

/**
 * @brief 销毁epoll句柄
 * 
 * @param handle 
 */
void epoll_timer_destroy(et_handle handle);

/**
 * @brief 添加定时器任务到监听事件
 * 
 * @param handle epoll句柄
 * @param task_info 任务指针
 * @return true 成功
 * @return false 失败
 */
bool epoll_timer_add_task(et_handle handle, const struct epoll_timer_task *task_info);

/**
 * @brief 从监听事件中移除定时器任务
 * 
 * @param handle epoll句柄
 * @param task_info 任务指针（需要匹配 f_entry）
 * @return true 成功
 * @return false 失败
 */
bool epoll_timer_remove_task(et_handle handle, const struct epoll_timer_task *task_info);

/**
 * @brief 停止事件监听
 * 
 * @param handle epoll句柄
 * @return true 成功
 * @return false 失败
 */
bool epoll_timer_stop(et_handle handle);

/**
 * @brief 轮询事件监听
 * 
 * @param handle epoll句柄
 * @return int 0:正常退出, 1:错误退出
 */
int epoll_timer_run(et_handle handle);

#endif /* _EPOLL_TIMER_H */
