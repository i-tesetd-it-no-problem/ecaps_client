# ECAPS(Edge Computing Analysis and Processing System 边缘计算分析与处理系统)

## 目录结构
- [简介](#1-简介)
- [功能介绍](#2-功能介绍)
- [编译说明](#3-编译说明)
- [运行说明(TODO)](#4-运行说明)

## 1. 简介
1. 本项目使用STM32MP157A开发板作为边缘计算设备端,实现数据采集、数据处理、数据传输功能。
使用服务器作为边缘计算数据中心,实现数据存储、数据分析、数据展示功能。

2. 本项目除代码以外用到的工具/插件有:
 - `git`            : 代码托管工具
 - `repo`           : git仓库管理工具 
 - `CMake`          : 编译管理工具
 - `ARM-GCC`        : 交叉编译工具链
 - `Unity`          : 单元测试工具
 - `clangd`         : 代码补全, 语法检查工具
 - `clang-format`   : 代码格式化工具
 - `openssl`        : 密钥, 证书生成工具
 - `CodeLLDB`       : 调试插件
 - `Doxygen`        : 文档生成插件

3. 本项目涉及到的技术栈有:
 - C语言
 - CMake语法
 - Linux内核驱动开发
 - Linux应用开发
 - mbedTLS网络变成
 - MarkDown文档语法
 - OPTEE CA/TA 可信应用开发

## 2. 功能介绍
### 2.1 设备端
#### 2.1.1 数据采集
- `lmv358传感器` 采集电压,电流
- `ap3216c传感器` 采集环境光/接近/红外
- `si7006传感器` 采集温湿度
- `rda226传感器` 采集人体红外
- `itr9608传感器` 采集光闸/火焰
- `max30102传感器` 采集心率血氧

#### 2.1.2 数据处理
使用`xxx`算法对采集到的数据进行处理,如滤波等。

#### 2.1.3 数据传输
- 使用`mbtdtls`第三方开源组件+`http`协议 与服务器进行数据交互
- 数据格式为`json`, 定义参考[JsonSchema定义](Docs/docs_third_part/cJson/JsonSchema.json)

### 2.2 服务端
##### 2.2.1 数据存储


#### 2.2.2 数据分析

##### 2.2.3 数据展示

## 3. 编译说明

### 3.1 编译须知
为了方便开发, 本项目设备端分为两种模式:
- 主机模式 : 只有部分功能, 即可直接在主机上编译运行测试, 缺少了与开发板相关功能, 如数据采集, OPTEE等, 仅作为开发测试使用, 开发板相关部分代码通过宏定义进行屏蔽,无需手动修改代码,只需要构建编译时提供一个参数`-DHOST_BUILD=XX`即可(后续会介绍)

- 开发板模式 : 拥有完整的功能

### 3.2 编译指南
这两种模式由于运行在不同的平台上, 因此需要使用两个不同的编译工具链

#### 3.2.1 编译第三方库
本项目所有的第三方库都安装在`third_part`目录下, 请确保提前在根目录下创建`third_part`目录
```shell
mkdir third_part
```

- 本项目使用mbedtls作为TLS协议栈与服务器进行通信, 需要手动编译安装, 具体请参考[mbedtls文档](Docs/docs_third_part/mbedtls/README.md)中的编译章节
- 本项目使用cJson作为json解析库, 需要手动编译安装, 具体请参考[cJson文档](Docs/docs_third_part/cJson/README.md)

#### 3.2.2 编译设备端
严格按照第三方库的编译指南进行编译之后, 即可编译设备端程序
用户可以按需修改[用户配置文件](include/user_config.h)
编译参考如下命令:
- 主机模式 : 
```shell
# 确保不存在build目录
rm -rf build;

# 创建编译配置
cmake -B build -S . -G Ninja \
-DCMAKE_TOOLCHAIN_FILE=toolchain_host.cmake \
-DHOST_BUILD=ON \
-DCMAKE_BUILD_TYPE=Debug
```
- 开发板模式 : 
```shell
# 确保不存在build目录
rm -rf build;

# 创建编译配置
cmake -B build -S . -G Ninja \
-DCMAKE_TOOLCHAIN_FILE=toolchain_board.cmake \
-DHOST_BUILD=OFF \
-DCMAKE_BUILD_TYPE=Debug
```

- 部分编译选项说明:
    - `CMAKE_TOOLCHAIN_FILE` : 指定使用的编译工具链, 这里使用的是`toolchain_host.cmake`和`toolchain_board.cmake`
    - `HOST_BUILD` : 开启主机模式, `ON`或`OFF` , 开启之后 源码中所有依赖开发板的代码都会被屏蔽, 如传感器采集相关
    - `CMAKE_BUILD_TYPE` : 编译类型, `Debug`或`Release`, 会使用不同的编译等级

## 4. 运行说明
TODO

在服务器运行如下仓库的代码`https://github.com/i-tesetd-it-no-problem/ecaps_server.git`

### 4.1 证书生成
本项目使用TLS协议与服务器进行通信, 因此需要在服务器上生成证书
服务端使用自建CA证书并同时为服务端和设备端生成证书, 具体请参考[服务端证书生成文档](Docs/docs_third_part/mbedtls/cert.md),请在服务端目录下执行该文档内容

### 4.2 复制证书
请将服务端生成的`ca.pem`,`client.crt`,`client.key`复制保存到本工程工具目录`tools/certification`下
在主机可使用如下命令进行测试
```shell
curl --cert tools/certification/client.crt --key tools/certification/client.key --cacert tools/certification/ca.pem https://xxxx/test
```
xxx替换为服务器公网IP

输出结果为 `{"message":"Mutual TLS connection successful"}` 则代表证书配置成功

## 5. 功能测试
本项目使用`Unity`工具进行单元测试, 具体参考[Unity](Docs/docs_third_part/unity/README.md)