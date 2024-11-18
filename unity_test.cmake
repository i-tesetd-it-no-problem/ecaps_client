# Unity 路径
set(UNITY_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_part/Unity/src)

# 设置 MBEDTLS 根目录
if(HOST_BUILD)
    set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/mbedtls/install_host)
else()
    set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/mbedtls/install_board)
endif()

# 启用测试功能
enable_testing()

# 添加 Unity 测试函数
function(add_unity_test test_name test_source)
    add_executable(${test_name} ${test_source} ${UNITY_ROOT}/unity.c)
    target_include_directories(${test_name} PRIVATE ${UNITY_ROOT})
    target_link_libraries(${test_name} PRIVATE pub_lib)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# 日志测试用例
add_unity_test(test_logger ${CMAKE_CURRENT_SOURCE_DIR}/test/test_logger.c)

# SSL 测试用例
add_unity_test(test_ssl_client ${CMAKE_CURRENT_SOURCE_DIR}/test/test_ssl_client.c)
target_link_libraries(test_ssl_client 
    PRIVATE pub_lib
    /home/wenshuyu/optee_client/out/libteec/libteec.a
    ${MBEDTLS_ROOT_DIR}/lib/libmbedtls.a
    ${MBEDTLS_ROOT_DIR}/lib/libmbedx509.a
    ${MBEDTLS_ROOT_DIR}/lib/libmbedcrypto.a
    ${MBEDTLS_ROOT_DIR}/lib/libeverest.a
    ${MBEDTLS_ROOT_DIR}/lib/libp256m.a
    ${MBEDTLS_ROOT_DIR}/lib/libbuiltin.a
)

# cJSON 测试用例
add_unity_test(test_cjson ${CMAKE_CURRENT_SOURCE_DIR}/test/test_cjson.c)
target_link_libraries(test_cjson PRIVATE pub_lib ${CJSON_ROOT_DIR}/lib/libcjson.a)
