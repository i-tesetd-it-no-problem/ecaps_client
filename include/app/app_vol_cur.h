/**
 * @file app_vol_cur.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 扩展版工作电压电流采集
 * @version 1.0
 * @date 2024-12-04
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

#ifndef _APP_VOL_CUR_H
#define _APP_VOL_CUR_H

// 采集电压和电流任务
void *coll_v_i_task(void *arg);

// 获取电压
float get_voltage(void);

// 获取电流
float get_current(void);

// 停止数据采集
void stop_coll_task(void);

#endif /* _APP_VOL_CUR_H */