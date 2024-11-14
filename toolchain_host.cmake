# 主机编译工具链, 通常不需要修改
# 没有的话 使用 sudo apt install build-essential 命令安装

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_ASM_COMPILER "/usr/bin/as")
set(CMAKE_AR "/usr/bin/ar")
set(CMAKE_SIZE "/usr/bin/size")
set(CMAKE_OBJCOPY "/usr/bin/objcopy")
set(CMAKE_OBJDUMP "/usr/bin/objdump")
set(CMAKE_RANLIB "/usr/bin/ranlib")
set(CMAKE_STRIP "/usr/bin/strip")

set(CMAKE_FIND_ROOT_PATH "")
set(CMAKE_SYSROOT "")