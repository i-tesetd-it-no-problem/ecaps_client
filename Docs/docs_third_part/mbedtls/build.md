# mbedTLS静态库编译

## 目录

- [下载mbedtls](#下载mbedtls)
- [开发板的编译环境](#开发板的编译环境)
- [主机的编译环境](#主机的编译环境)

### 下载mbedtls
```sh
# 进入第三方库目录
cd third_part

# 下载mbedtls
git clone https://github.com/ARMmbed/mbedtls.git

# 进入mbedtls目录
cd mbedtls

# 更新依赖的子模块
git submodule update --init --recursive
```

### 开发板的编译环境

```sh
# 确保已经删除掉build和install_board目录
rm -rf build;rm -rf install_board

# 指定工具链与安装目录
cmake .. \
  -B build -S . -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=../../toolchain_board.cmake \
  -DCMAKE_INSTALL_PREFIX=./install_board
  
# 编译静态库到install_board目录
cmake --build build --target install
```

### 主机的编译环境
```sh
# 确保已经删除掉build和install_host目录
rm -rf build;rm -rf install_host;

# 指定工具链与安装目录
cmake .. \
  -B build -S . -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=../../toolchain_host.cmake \
  -DCMAKE_INSTALL_PREFIX=./install_host

# 编译静态库到install_host目录
cmake --build build --target install
```

本项目根目录下的CMakeLists.txt中已经配置了mbedtls的链接选项, 请勿修改上述命令参数