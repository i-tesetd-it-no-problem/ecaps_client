#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/* 日志相关配置 */
#define DEFAULT_LOG_FILE        "/tmp/ecaps.log"                    // 日志文件路径(开启相关接口才有效, 默认输出到控制台)
#define LOG_MAX_SIZE            (512)                               // 日志消息最大长度

/* 本地证书路径相关配置, 只有在主机执行环境才有效 */
#define REL_CA_PEM_PATH         "tools/certification/ca.pem"        // ca.pem的工程相对路径
#define REL_CLIENT_CRT_PATH     "tools/certification/client.crt"    // client.crt的工程相对路径
#define REL_CLIENT_KEY_PATH     "tools/certification/client.key"    // client.key的工程相对路径

/* OPTEE 可信存储配置, 只有在开发板执行环境才有效 */
#define CA_PEM_OBJ_ID           "ca.pem"                            // ca.pem的可信存储对象ID
#define CLIENT_CER_OBJ_ID       "client.crt"                        // client.crt的可信存储对象ID
#define CLIENT_KEY_OBJ_ID       "client.key"                        // client.key的可信存储对象ID

// 为了把证书存储到可信环境中, 需要先把证书拷贝到开发板上，再调用接口保存
#define BOARD_CA_PEM_PATH       "/home/wenshuyu/ca.pem"             // 开发板上的ca.pem路径
#define BOARD_CLIENT_CER_PATH   "/home/wenshuyu/client.crt"         // 开发板上的client.crt路径
#define BOARD_CLIENT_KEY_PATH   "/home/wenshuyu/client.key"         // 开发板上的client.key路径

#endif /* __USER_CONFIG_H__ */