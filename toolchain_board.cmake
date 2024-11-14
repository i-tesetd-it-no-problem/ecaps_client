#################################修改区域#################################
# 系统名称和处理器架构
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7-a)

# 工具链根路径
set(TOOLCHAIN_ROOT "/opt/st/stm32mp1/4.2.4-openstlinux-6.1-yocto-mickledore-mpu-v24.06.26")

# 工具链目录和前缀
set(TOOLCHAIN_BIN "${TOOLCHAIN_ROOT}/sysroots/x86_64-ostl_sdk-linux/usr/bin/arm-ostl-linux-gnueabi")
set(TOOLCHAIN_PREFIX "arm-ostl-linux-gnueabi")

# 编译器标志
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_SYSROOT} -mfloat-abi=hard -mfpu=vfpv3-d16")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_SYSROOT} -mfloat-abi=hard -mfpu=vfpv3-d16")
#################################修改区域#################################


#################################固定区域#################################
# 设置工具链
set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-as")
set(CMAKE_SIZE "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-size")
set(CMAKE_AR "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-ar")
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-objdump")
set(CMAKE_RANLIB "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-ranlib")
set(CMAKE_STRIP "${TOOLCHAIN_BIN}/${TOOLCHAIN_PREFIX}-strip")

# 查找库和头文件的根路径
set(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_ROOT}/sysroots/cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi")

# 优先从工具链路径查找
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 设置sysroot
set(CMAKE_SYSROOT "${CMAKE_FIND_ROOT_PATH}")
#################################固定区域#################################