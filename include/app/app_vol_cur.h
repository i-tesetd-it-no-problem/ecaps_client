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