enable_testing()

########################### 日志测试用例 ###########################
add_executable(test_logger ${CMAKE_CURRENT_SOURCE_DIR}/test/test_logger.c)
target_link_libraries(test_logger PRIVATE pub_lib)
add_test(NAME LoggerTest COMMAND test_logger)


########################### SSL测试用例 ###########################
if(HOST_BUILD)
    set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/mbedtls/install_host)
else()
    set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/mbedtls/install_board)
endif()

add_executable(test_ssl_client ${CMAKE_CURRENT_SOURCE_DIR}/test/test_ssl_client.c)
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
add_test(NAME SSLTest COMMAND test_ssl_client)

########################### cJson测试用例 ###########################
if(HOST_BUILD)
    set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/mbedtls/install_host)
else()
    set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_part/mbedtls/install_board)
endif()

add_executable(test_cjson ${CMAKE_CURRENT_SOURCE_DIR}/test/test_cjson.c)
target_link_libraries(test_cjson PRIVATE pub_lib ${CJSON_ROOT_DIR}/lib/libcjson.a)
add_test(NAME CJSONTest COMMAND test_cjson)