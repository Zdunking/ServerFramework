cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_COMPILER "/apps/stone/bin/gcc")
set(CMAKE_CXX_COMPILER "/apps/stone/bin/g++")

project(zdunk)

#include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP})

set(LIB_SRC
    zdunk/log.cc
    zdunk/utils.cc
    zdunk/config.cc
    zdunk/thread.cc
    zdunk/fiber.cc
    zdunk/scheduler.cc
    zdunk/iomanager.cc
    zdunk/timer.cc
    zdunk/fd_manager.cc
    zdunk/hook.cc
    zdunk/address.cc
    zdunk/socket.cc
    zdunk/bytearray.cc
    zdunk/http/http.cc
    zdunk/http/http11_parser.rl.cc
    zdunk/http/httpclient_parser.rl.cc
    zdunk/http/http_parser.cc
    zdunk/tcp_server.cc
    zdunk/stream.cc
    zdunk/stream/socket_stream.cc
    zdunk/http/http_session.cc
    zdunk/http/http_server.cc
    )

add_library(zdunk SHARED ${LIB_SRC})
#add_library(zdunk_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(zdunk_static PROPERTIES OUTPUT_NAME "zdunk")

set(
    LIB_LIB
    zdunk
    pthread
    dl
    ${YAMLCPP}
)

add_executable(test tests/test.cc)
add_dependencies(test zdunk)
target_link_libraries(test ${LIB_LIB})

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config zdunk)
target_link_libraries(test_config ${LIB_LIB})

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread zdunk)
target_link_libraries(test_thread ${LIB_LIB})

add_executable(test_utils tests/test_utils.cc)
add_dependencies(test_utils zdunk)
target_link_libraries(test_utils ${LIB_LIB})

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber zdunk)
target_link_libraries(test_fiber ${LIB_LIB})

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler zdunk)
target_link_libraries(test_scheduler ${LIB_LIB})

add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager zdunk)
target_link_libraries(test_iomanager ${LIB_LIB})

add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook zdunk)
target_link_libraries(test_hook ${LIB_LIB})

add_executable(test_address tests/test_address.cc)
add_dependencies(test_address zdunk)
target_link_libraries(test_address ${LIB_LIB})

add_executable(test_socket tests/test_socket.cc)
add_dependencies(test_socket zdunk)
target_link_libraries(test_socket ${LIB_LIB})

add_executable(test_bytearray tests/test_bytearray.cc)
add_dependencies(test_bytearray zdunk)
target_link_libraries(test_bytearray ${LIB_LIB})

add_executable(test_http tests/test_http.cc)
add_dependencies(test_http zdunk)
target_link_libraries(test_http ${LIB_LIB})

add_executable(test_http_parser tests/test_http_parser.cc)
add_dependencies(test_http_parser zdunk)
target_link_libraries(test_http_parser ${LIB_LIB})

add_executable(test_tcp_server tests/test_tcp_server.cc)
add_dependencies(test_tcp_server zdunk)
target_link_libraries(test_tcp_server ${LIB_LIB})

add_executable(echo_server examples/echo_server.cc)
add_dependencies(echo_server zdunk)
target_link_libraries(echo_server ${LIB_LIB})

add_executable(test_http_server tests/test_http_server.cc)
add_dependencies(test_http_server zdunk)
target_link_libraries(test_http_server ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)