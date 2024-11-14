# cJson编译说明

## 目录
- [下载cJSON源码](#下载cjson源码)
- [开发板环境](#开发板环境)
- [主机环境](#主机环境)
- [部分编译选项](#部分编译选项)

### 下载cJSON源码
```sh
# 进入第三方库目录
cd third_part

# 下载cJSON源码
git clone https://github.com/DaveGamble/cJSON.git

# 进入cJSON源码目录
cd cJSON
```

### 开发板环境
```shell
# 确保已经删除掉build和install_board目录
rm -rf install_board;rm -rf build

# 指定工具链与安装目录
cmake .. \
  -B build -S . -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=../../toolchain_board.cmake \
  -DCMAKE_INSTALL_PREFIX=./install_board \
  -DBUILD_SHARED_AND_STATIC_LIBS=On \
  -DENABLE_CJSON_TEST=Off \
  -DENABLE_CJSON_UTILS=Off

# 编译
cmake --build build --target install
```

### 主机环境
```shell
# 确保已经删除掉build和install_host目录
rm -rf install_host;rm -rf build

# 指定工具链与安装目录
cmake .. \
  -B build -S . -G Ninja \
  -DCMAKE_INSTALL_PREFIX=./install_host \
  -DBUILD_SHARED_AND_STATIC_LIBS=On \
  -DENABLE_CJSON_TEST=Off \
  -DENABLE_CJSON_UTILS=Off

# 编译
cmake --build build --target install
```

## 部分编译选项
- `-DBUILD_SHARED_AND_STATIC_LIBS=On` 编译生成共享库和静态库
- `-DENABLE_CJSON_TEST=Off` 关闭测试用例
- `-DENABLE_CJSON_UTILS=Off` 关闭工具
- `-DCMAKE_INSTALL_PREFIX=./install_host` 指定安装路径
