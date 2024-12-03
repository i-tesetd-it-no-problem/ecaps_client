# 最低版本要求
cmake_minimum_required(VERSION 3.20)

# 策略
cmake_policy(SET CMP0123 NEW)

# 工程宏定义
include(${CMAKE_CURRENT_SOURCE_DIR}/macro.cmake)

# 项目名称和类型
project("ECAPS" C)

# 设置生成 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE "-O2")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS_DEBUG "-g")
endif()

# 编译输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/output)

# 第三方库路径配置
if(HOST_BUILD)
    set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/mbedtls/install_host)
    set(CJSON_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/cJSON/install_host)
else()
    set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/mbedtls/install_board)
    set(CJSON_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/cJSON/install_board)
endif()

# 包含所有头文件路径
set(TOP_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /home/wenshuyu/optee_client/libteec/include
    ${MBEDTLS_ROOT_DIR}/include
    ${CJSON_ROOT_DIR}/include
)
include_directories(${TOP_INCLUDE_DIRS})

# 工程源文件目录
file(GLOB_RECURSE TOP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c)

# 共享静态库
add_library(pub_lib STATIC ${TOP_SOURCE_FILES})
target_include_directories(pub_lib PRIVATE ${TOP_INCLUDE_DIRS})

# 主工程
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/source/app/main.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_INCLUDE_DIRS})

# 链接库
target_link_libraries(${PROJECT_NAME} 
    PRIVATE pub_lib                                         # 工程源文件静态库(不包括可执行文件)

    # OPTEE Client 静态库
    /home/wenshuyu/optee_client/out/libteec/libteec.a

    # mbed 静态库
    ${MBEDTLS_ROOT_DIR}/lib/libmbedtls.a
    ${MBEDTLS_ROOT_DIR}/lib/libmbedx509.a
    ${MBEDTLS_ROOT_DIR}/lib/libmbedcrypto.a
    ${MBEDTLS_ROOT_DIR}/lib/libeverest.a
    ${MBEDTLS_ROOT_DIR}/lib/libp256m.a
    ${MBEDTLS_ROOT_DIR}/lib/libbuiltin.a
    
    #cJSON 静态库
    ${CJSON_ROOT_DIR}/lib/libcjson.a
)

# 测试用例
include(${CMAKE_CURRENT_SOURCE_DIR}/unity_test.cmake)

# 自定义命令
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}
)

# 主机
# rm -rf build;cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_host.cmake -DHOST_BUILD=ON -DCMAKE_BUILD_TYPE=Debug
# rm -rf build;cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_host.cmake -DHOST_BUILD=ON -DCMAKE_BUILD_TYPE=Release

# 开发板
# rm -rf build;cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_board.cmake -DHOST_BUILD=OFF -DCMAKE_BUILD_TYPE=Debug
# rm -rf build;cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_board.cmake -DHOST_BUILD=OFF -DCMAKE_BUILD_TYPE=Release

# cmake --build build --target clean
# scp ./build/output/ECAPS wenshuyu@192.168.1.6:/home/wenshuyu/ecaps_client
# scp ./build/output/test_logger wenshuyu@192.168.1.6:/home/wenshuyu/ecaps_client