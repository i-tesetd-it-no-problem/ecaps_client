
# 各个环境下的宏定义

if(HOST_BUILD)
    # 主机环境编译
else()
    # 交叉编译环境编译
    add_compile_definitions(BOARD_ENV)
    
endif()

add_compile_definitions(PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}") # 项目根目录