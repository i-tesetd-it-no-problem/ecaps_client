#include "utils/epoll_timer.h"
#include "utils/logger.h"

#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/eventfd.h>

// 最大事件数
#ifndef MAX_EVENTS
#define MAX_EVENTS 64
#endif

// 函数指针
struct task_f {
	timer_task_init f_init;		// 初始化函数
	timer_task_entry f_entry;	// 任务处理函数
	timer_task_deinit f_deinit; // 去初始化函数
};

// 任务实例
struct timer_task {
	struct task_f *task_f; // 函数指针
	size_t period_ms;	   // 任务周期
	int timer_fd;		   // 定时器描述符

	void *priv; // 私有数据
	struct timer_task *prev;
	struct timer_task *next;
};

// epoll_timer结构
struct epoll_timer {
	struct timer_task *task_list; // 任务链表
	int epoll_fd;				  // 事件描述符
	int stop_eventfd;			  // 停止任务描述符
	pthread_mutex_t lock;		  // 互斥锁
	bool running;				  // 运行标志
};

/**
 * @brief 创建epoll监听句柄
 * 
 * @return et_handle 失败返回NULL，成功返回句柄
 */
et_handle epoll_timer_create(void)
{
	// 申请句柄
	et_handle handle = malloc(sizeof(struct epoll_timer));
	if (!handle) {
		LOG_E("Failed to allocate memory for epoll_timer.");
		return NULL;
	}
	memset(handle, 0, sizeof(struct epoll_timer));

	// 初始化互斥锁
	if (pthread_mutex_init(&handle->lock, NULL) != 0) {
		LOG_E("Failed to initialize mutex.");
		goto err_free_handle;
	}

	// 创建epoll实例，使用EPOLL_CLOEXEC
	handle->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if (handle->epoll_fd < 0) {
		LOG_E("Failed to create epoll instance: %s", strerror(errno));
		goto err_free_mutex;
	}

	// 创建停止事件fd
	handle->stop_eventfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (handle->stop_eventfd < 0) {
		LOG_E("Failed to create stop eventfd: %s", strerror(errno));
		goto err_free_epoll;
	}

	// 停止事件fd添加到epoll
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = handle->stop_eventfd;
	if (epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, handle->stop_eventfd, &ev) < 0) {
		LOG_E("Failed to add stop_eventfd to epoll: %s", strerror(errno));
		goto err_free_stop_fd;
	}

	handle->task_list = NULL;
	handle->running = false;

	return handle;

// 错误处理
err_free_handle:
	free(handle);

err_free_mutex:
	pthread_mutex_destroy(&handle->lock);

err_free_epoll:
	close(handle->epoll_fd);

err_free_stop_fd:
	close(handle->stop_eventfd);

	return NULL;
}

/**
 * @brief 销毁epoll句柄
 * 
 * @param handle 
 */
void epoll_timer_destroy(et_handle handle)
{
	if (!handle)
		return;

	// 先停止事件监听
	epoll_timer_stop(handle);

	// 关闭停止事件fd
	close(handle->stop_eventfd);

	// 关闭epoll描述符
	if (handle->epoll_fd >= 0)
		close(handle->epoll_fd);

	pthread_mutex_lock(&handle->lock);

	// 遍历任务链表并销毁每个任务
	struct timer_task *task = handle->task_list;
	while (task) {
		struct timer_task *next_task = task->next;

		if (task->task_f) {
			if (task->task_f->f_deinit)
				task->task_f->f_deinit(task->priv);

			free(task->task_f);
		}

		if (task->timer_fd >= 0)
			close(task->timer_fd);

		free(task);

		task = next_task;
	}

	handle->task_list = NULL;
	pthread_mutex_unlock(&handle->lock);

	pthread_mutex_destroy(&handle->lock); // 销毁互斥锁

	free(handle); // 释放句柄
}

/**
 * @brief 添加定时器任务到监听事件
 * 
 * @param handle epoll句柄
 * @param task_info 任务指针
 * @return true 成功
 * @return false 失败
 */
bool epoll_timer_add_task(et_handle handle, const struct epoll_timer_task *task_info)
{
	if (!handle || !task_info) {
		LOG_E("Invalid arguments to epoll_timer_add_task.");
		return false;
	}

	// 创建任务实例
	struct timer_task *new_task = malloc(sizeof(struct timer_task));
	if (!new_task) {
		LOG_E("Failed to allocate memory for new task.");
		return false;
	}
	memset(new_task, 0, sizeof(struct timer_task));
	new_task->timer_fd = -1;

	// 任务初始化
	if (task_info->f_init) {
		if (!task_info->f_init(&new_task->priv)) {
			LOG_E("Task initialization failed.");
			goto err_free_new_task;
		}
	}

	// 如果周期为0或没有任务处理函数，只进行初始化
	if (task_info->period_ms == 0 || task_info->f_entry == NULL) {
		LOG_I("Task has no entry function or zero period, only ran initialization.");

		// 添加到任务链表
		pthread_mutex_lock(&handle->lock);
		new_task->next = handle->task_list;
		if (handle->task_list)
			handle->task_list->prev = new_task;
		handle->task_list = new_task;
		pthread_mutex_unlock(&handle->lock);

		return true; // 返回成功
	}

	// 分配任务函数结构
	new_task->task_f = malloc(sizeof(struct task_f));
	if (!new_task->task_f) {
		LOG_E("Failed to allocate memory for task functions.");
		goto err_free_new_task;
	}

	// 设置任务的函数指针和周期
	new_task->task_f->f_init = task_info->f_init;
	new_task->task_f->f_entry = task_info->f_entry;
	new_task->task_f->f_deinit = task_info->f_deinit;
	new_task->period_ms = task_info->period_ms;

	// 创建定时器文件描述符
	new_task->timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (new_task->timer_fd < 0) {
		LOG_E("Failed to create timerfd: %s", strerror(errno));
		goto err_free_task_f;
	}

	// 设置定时器的周期和初始时间
	struct itimerspec timer_spec;
	memset(&timer_spec, 0, sizeof(timer_spec));
	timer_spec.it_interval.tv_sec = task_info->period_ms / 1000;
	timer_spec.it_interval.tv_nsec = (task_info->period_ms % 1000) * 1000000;
	timer_spec.it_value.tv_sec = task_info->period_ms / 1000;
	timer_spec.it_value.tv_nsec = (task_info->period_ms % 1000) * 1000000;

	// 设置定时器
	if (timerfd_settime(new_task->timer_fd, 0, &timer_spec, NULL) < 0) {
		LOG_E("Failed to set timerfd: %s", strerror(errno));
		goto err_close_timer_fd;
	}

	// 注册定时器到epoll
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN | EPOLLET;
	ev.data.ptr = new_task; // 保存指针

	// 添加监听
	if (epoll_ctl(handle->epoll_fd, EPOLL_CTL_ADD, new_task->timer_fd, &ev) < 0) {
		LOG_E("Failed to add timerfd to epoll: %s", strerror(errno));
		goto err_close_timer_fd;
	}

	// 添加到任务链表
	pthread_mutex_lock(&handle->lock);
	new_task->next = handle->task_list;
	if (handle->task_list)
		handle->task_list->prev = new_task;
	handle->task_list = new_task;
	pthread_mutex_unlock(&handle->lock);

	return true; // 返回成功

// 错误处理
err_close_timer_fd:
	if (new_task->timer_fd >= 0)
		close(new_task->timer_fd);

err_free_task_f:
	if (new_task->task_f)
		free(new_task->task_f);

err_free_new_task:
	if (task_info->f_deinit)
		task_info->f_deinit(new_task->priv);

	free(new_task);

	return false;
}

/**
 * @brief 从监听事件中移除定时器任务
 * 
 * @param handle epoll句柄
 * @param task_info 任务指针（需要匹配 f_entry）
 * @return true 成功
 * @return false 失败
 */
bool epoll_timer_remove_task(et_handle handle, const struct epoll_timer_task *task_info)
{
	if (!handle || !task_info) {
		LOG_E("Invalid arguments to epoll_timer_remove_task.");
		return false;
	}

	// 仅当 f_entry 非 NULL 时才能移除任务
	if (task_info->f_entry == NULL) {
		LOG_E("f_entry is NULL in epoll_timer_remove_task. Unable to identify the task.");
		return false;
	}

	pthread_mutex_lock(&handle->lock);
	struct timer_task *task = handle->task_list;
	while (task) {
		if (task->task_f && task->task_f->f_entry == task_info->f_entry) {
			// 移除
			if (task->timer_fd >= 0) {
				if (epoll_ctl(handle->epoll_fd, EPOLL_CTL_DEL, task->timer_fd, NULL) < 0) {
					LOG_E("Failed to remove timerfd from epoll: %s", strerror(errno));
				}
				close(task->timer_fd);
			}

			// 调用去初始化函数
			if (task->task_f->f_deinit)
				task->task_f->f_deinit(task->priv);

			// 移除任务链表
			if (task->prev)
				task->prev->next = task->next;
			else
				handle->task_list = task->next;

			if (task->next)
				task->next->prev = task->prev;

			// 释放资源
			free(task->task_f);
			free(task);

			pthread_mutex_unlock(&handle->lock);
			return true;
		}
		task = task->next;
	}
	pthread_mutex_unlock(&handle->lock);

	LOG_E("Task not found for removal.");
	return false;
}

/**
 * @brief 停止事件监听
 * 
 * @param handle epoll句柄
 * @return true 成功
 * @return false 失败
 */
bool epoll_timer_stop(et_handle handle)
{
	if (!handle || handle->stop_eventfd < 0) {
		LOG_E("Invalid handle in epoll_timer_stop.");
		return false;
	}

	uint64_t stop_signal = 1;
	ssize_t s = write(handle->stop_eventfd, &stop_signal, sizeof(stop_signal));
	if (s != sizeof(stop_signal)) {
		LOG_E("Failed to send stop signal: %s", strerror(errno));
		return false;
	}

	return true;
}

/**
 * @brief 轮询事件监听
 * 
 * @param handle epoll句柄
 * @return int 0:正常退出, 1:错误退出
 */
int epoll_timer_run(et_handle handle)
{
	if (!handle) {
		LOG_E("Invalid handle in epoll_timer_run.");
		return 1;
	}

	// 已经运行直接返回
	pthread_mutex_lock(&handle->lock);
	if (handle->running) {
		pthread_mutex_unlock(&handle->lock);
		LOG_E("epoll_timer_run is already running.");
		return 1;
	}
	handle->running = true;
	pthread_mutex_unlock(&handle->lock);

	struct epoll_event events[MAX_EVENTS]; // 最大监听事件

	while (1) {
		// 等待事件触发
		int nfds = epoll_wait(handle->epoll_fd, events, MAX_EVENTS, -1);
		if (nfds < 0) {
			// 异常唤醒
			if (errno == EINTR)
				continue;
			LOG_E("epoll_wait failed: %s", strerror(errno));
			break;
		}

		if (nfds == 0) {
			// 理论上不会发生，因为超时为-1
			LOG_W("epoll_wait returned 0, unexpected with infinite timeout.");
			continue;
		}

		for (int i = 0; i < nfds; i++) {
			// 收到停止事件
			if (events[i].data.fd == handle->stop_eventfd) {
				uint64_t buf;
				ssize_t s = read(handle->stop_eventfd, &buf, sizeof(buf));
				if (s < 0 && errno != EAGAIN)
					LOG_E("Failed to read stop eventfd: %s", strerror(errno));

				LOG_I("Received stop signal. Exiting run loop.");
				pthread_mutex_lock(&handle->lock);
				handle->running = false;
				pthread_mutex_unlock(&handle->lock);
				return 0; // 0 正常返回
			}

			// 可以读事件
			if (events[i].events & EPOLLIN) {
				struct timer_task *task = (struct timer_task *)events[i].data.ptr;
				if (!task) {
					LOG_E("Invalid task pointer in epoll event.");
					continue;
				}

				uint64_t expirations;
				ssize_t s = read(task->timer_fd, &expirations, sizeof(expirations));
				if (s != sizeof(expirations)) {
					LOG_E("Failed to read timerfd: %s", strerror(errno));
					continue;
				}

				if (task->task_f && task->task_f->f_entry)
					task->task_f->f_entry(task->priv); // 执行任务
			}
		}
	}

	pthread_mutex_lock(&handle->lock);
	handle->running = false;
	pthread_mutex_unlock(&handle->lock);
	return 1; // 1:错误退出
}
