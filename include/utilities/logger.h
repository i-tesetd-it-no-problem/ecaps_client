/**
 * @file logger.h
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

#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define DEFAULT_LOG_FILE "/tmp/ecaps.log"   // 默认日志文件路径
#define LOG_MESSAGE_MAX_SIZE 512            // 日志消息最大长度

#ifdef PROJECT_ROOT
static inline const char* __get_rel_(const char* file) {
    const char* root = strstr(file, PROJECT_ROOT);
    return root ? file + strlen(PROJECT_ROOT) : file;
}
#define RELATIVE_FILE __get_rel_(__FILE__)
#else
#define RELATIVE_FILE __FILE__
#endif

enum log_level{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE
};

/**
 * @brief 输出日志
 * 
 * @param level 日志级别
 * @param file 当前文件
 * @param line 当前行号
 * @param fmt 日志格式
 * @param ... 日志参数
 */
void logger_log(enum log_level level, const char *file, int line, const char *fmt, ...);

/**
 * @brief 是否输出日志到文件
 * 
 * @param to_file 
 * @return true 输出到 宏 DEFAULT_LOG_FILE
 * @return false 输出到控制台
 */
bool logger_set_output(bool to_file);

/**
 * @brief 设置日志级别
 * 
 * @param level 
 * @return true 成功
 * @return false 失败
 */
bool logger_set_level(enum log_level level);

/**
 * @brief 日志中是否显示时间戳
 * 
 * @param enable 
 */
void logger_enable_timestamp(bool enable);

// 日志宏
#define LOG_D(fmt, ...) logger_log(LOG_LEVEL_DEBUG, RELATIVE_FILE, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) logger_log(LOG_LEVEL_INFO,  RELATIVE_FILE, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) logger_log(LOG_LEVEL_WARN,  RELATIVE_FILE, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) logger_log(LOG_LEVEL_ERROR, RELATIVE_FILE, __LINE__, fmt, ##__VA_ARGS__)

#endif /* LOGGER_H */
