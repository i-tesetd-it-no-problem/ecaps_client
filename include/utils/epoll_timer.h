#ifndef _EPOLL_TIMER_H
#define _EPOLL_TIMER_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Epoll定时器系统句柄
 */
typedef struct epoll_timer *et_handle;

/**
 * @brief 定时任务初始化函数。成功时返回true。
 */
typedef bool (*timer_task_init)(void);

/**
 * @brief 定时任务处理函数。
 */
typedef void (*timer_task_entry)(void);

/**
 * @brief 定时任务销毁函数，用于释放资源。
 */
typedef void (*timer_task_deinit)(void);

/**
 * @brief 定时任务信息
 */
struct epoll_timer_task {
	timer_task_init f_init;		// 可为NULL
	timer_task_entry f_entry;	// 可为NULL
	timer_task_deinit f_deinit; // 可为NULL
	size_t period_ms;			// 任务周期(毫秒)
};

/**
 * @brief 创建epoll监听句柄
 * 
 * @return et_handle 失败返回NULL 成功返回句柄
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
 * @return true 
 * @return false 
 */
bool epoll_timer_add_task(et_handle handle, const struct epoll_timer_task *task_info);

/**
 * @brief 从监听事件中移除定时器任务
 * 
 * @param handle epoll句柄
 * @param task_info 任务指针
 * @return true 
 * @return false 
 */
bool epoll_timer_remove_task(et_handle handle, const struct epoll_timer_task *task_info);

/**
 * @brief 轮训事件监听
 * 
 * @param handle epoll句柄
 * @return int 0:正常退出 1:错误退出
 */
int epoll_timer_run(et_handle handle);

/**
 * @brief 停止事件监听
 * 
 * @param handle epoll句柄
 * @return true 
 * @return false 
 */
bool epoll_timer_stop(et_handle handle);

#endif /* _EPOLL_TIMER_H */
