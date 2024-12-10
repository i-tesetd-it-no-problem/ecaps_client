/**
 * @file logger.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 日志模块
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
#include <pthread.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "utils/logger.h"
#include "user_config.h"

#define LOG_FINAL_MAX_SIZE (LOG_MAX_SIZE + 256)

typedef struct {
    enum log_level level;
    int log_fd;
    bool timestamp_enabled;
    bool use_file_output;
    pthread_mutex_t lock;
} logger_t;

static logger_t g_logger = {
    .level = LOG_LEVEL_INFO,    // 默认日志等级为INFO
    .log_fd = STDOUT_FILENO,
    .timestamp_enabled = true,
    .use_file_output = false,   // 默认使用标准输出
    .lock = PTHREAD_MUTEX_INITIALIZER
};

static __thread pid_t cached_thread_id = 0;

static pid_t get_thread_id() {
    if (cached_thread_id == 0) 
        cached_thread_id = syscall(SYS_gettid);

    return cached_thread_id;
}

static bool get_current_time(char *buffer, size_t size)
{
    if (!buffer || size == 0) 
        return false;
    
    time_t now = time(NULL);
    struct tm tm_info;
    if (localtime_r(&now, &tm_info) == NULL) 
        return false;
    
    if (strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info) == 0) 
        return false;
    
    return true;
}

static const char* level_to_string(enum log_level level)
{
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO ";
        case LOG_LEVEL_WARN:  return "WARN ";
        case LOG_LEVEL_ERROR: return "ERROR";
        default: return "UNKWN";
    }
}

/************************API************************/
bool logger_set_output(bool to_file)
{
    pthread_mutex_lock(&g_logger.lock);

    if (to_file && g_logger.log_fd == STDOUT_FILENO) {
        int fd = open(DEFAULT_LOG_FILE, O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd < 0) {
            perror("Failed to open log file");
            pthread_mutex_unlock(&g_logger.lock);
            return false;
        }
        g_logger.log_fd = fd;
    }

    else if (!to_file && g_logger.log_fd != STDOUT_FILENO) {
        close(g_logger.log_fd);
        g_logger.log_fd = STDOUT_FILENO;
    }

    g_logger.use_file_output = to_file;
    pthread_mutex_unlock(&g_logger.lock);
    return true;
}

bool logger_set_level(enum log_level level)
{
    bool result = true;
    pthread_mutex_lock(&g_logger.lock);

    if (level >= LOG_LEVEL_DEBUG && level <= LOG_LEVEL_NONE) 
        g_logger.level = level;
    else 
        result = false;

    pthread_mutex_unlock(&g_logger.lock);
    return result;
}

void logger_enable_timestamp(bool enable)
{
    pthread_mutex_lock(&g_logger.lock);
    g_logger.timestamp_enabled = enable;
    pthread_mutex_unlock(&g_logger.lock);
}

void logger_log(enum log_level level, const char *file, int line, const char *fmt, ...)
{
    if (level < g_logger.level || level == LOG_LEVEL_NONE || !fmt)
        return;

    char time_str[20] = "";
    if (g_logger.timestamp_enabled && !get_current_time(time_str, sizeof(time_str))) {
        strncpy(time_str, "UNKNOWN_TIME", sizeof(time_str) - 1);
        time_str[sizeof(time_str) - 1] = '\0';
    }

    pid_t thread_id = get_thread_id();

    char message_buffer[LOG_MAX_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);
    va_end(args);

    char final_log[LOG_FINAL_MAX_SIZE];
    if (g_logger.timestamp_enabled) {
        snprintf(final_log, sizeof(final_log), "[%s] [%-5s] [TID:%-3d] [%-20s:%-4d] %s\n",
                 time_str, level_to_string(level), thread_id, file, line, message_buffer);
    } else {
        snprintf(final_log, sizeof(final_log), "[%-5s] [TID:%-3d] [%-20s:%-4d] %s\n",
                 level_to_string(level), thread_id, file, line, message_buffer);
    }

    pthread_mutex_lock(&g_logger.lock);
    ssize_t bytes_written = write(g_logger.log_fd, final_log, strlen(final_log));
    if (bytes_written < 0) 
        fprintf(stderr, "write message failed\n");

    pthread_mutex_unlock(&g_logger.lock);
}
