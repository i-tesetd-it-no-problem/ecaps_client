#ifndef _APP_UPLOAD_H
#define _APP_UPLOAD_H

#include <stdbool.h>

// #define APP_UPLOAD_TASK_PERIOD (60 * 1000) // 1分钟
#define APP_UPLOAD_TASK_PERIOD (1000)

bool app_upload_init(void);
void app_upload_deinit(void);
void app_upload_task(void);

#endif /* _APP_UPLOAD_H */