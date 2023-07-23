cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_COMPILER "/apps/stone/bin/gcc")
set(CMAKE_CXX_COMPILER "/apps/stone/bin/g++")

project(zdunk)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP})

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(LIB_SRC
    zdunk/log.cc
    zdunk/utils.cc
    zdunk/config.cc
    zdunk/env.cc
    zdunk/db/fox_thread.cc
    zdunk/db/mysql.cc
    zdunk/db/sqlite3.cc
    zdunk/thread.cc
    zdunk/fiber.cc
    zdunk/scheduler.cc
    zdunk/iomanager.cc
    zdunk/worker.cc
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
    zdunk/http/http_connection.cc
    zdunk/http/http_server.cc
    zdunk/http/servlet.cc
    zdunk/uri.rl.cc
    zdunk/daemon.cc
    zdunk/application.cc
    zdunk/library.cc
    zdunk/module.cc
    zdunk/util/crypto_util.cc
    zdunk/util/json_util.cc
    zdunk/util/hash_util.cc
    )


add_library(zdunk SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(zdunk)
#add_library(zdunk_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(zdunk_static PROPERTIES OUTPUT_NAME "zdunk")

set(LIBS
    zdunk
    pthread
    dl
    ${YAMLCPP}
    ${OPENSSL_LIBRARIES}
    event
    jsoncpp
    mysqlclient_r
    sqlite3
    tinyxml2
)

zdunk_add_executable(test tests/test.cc zdunk "${LIBS}")
zdunk_add_executable(test_config tests/test_config.cc zdunk "${LIBS}")
zdunk_add_executable(test_thread tests/test_thread.cc zdunk "${LIBS}")
zdunk_add_executable(test_utils tests/test_utils.cc zdunk "${LIBS}")
zdunk_add_executable(test_fiber tests/test_fiber.cc zdunk "${LIBS}")
zdunk_add_executable(test_scheduler tests/test_scheduler.cc zdunk "${LIBS}")
zdunk_add_executable(test_iomanager tests/test_iomanager.cc zdunk "${LIBS}")
zdunk_add_executable(test_hook tests/test_hook.cc zdunk "${LIBS}")
zdunk_add_executable(test_address tests/test_address.cc zdunk "${LIBS}")
zdunk_add_executable(test_socket tests/test_socket.cc zdunk "${LIBS}")
zdunk_add_executable(test_bytearray tests/test_bytearray.cc zdunk "${LIBS}")
zdunk_add_executable(test_http tests/test_http.cc zdunk "${LIBS}")
zdunk_add_executable(test_http_parser tests/test_http_parser.cc zdunk "${LIBS}")
zdunk_add_executable(test_tcp_server tests/test_tcp_server.cc zdunk "${LIBS}")
zdunk_add_executable(echo_server examples/echo_server.cc zdunk "${LIBS}")
zdunk_add_executable(test_http_server tests/test_http_server.cc zdunk "${LIBS}")
zdunk_add_executable(test_http_connection tests/test_http_connection.cc zdunk "${LIBS}")
zdunk_add_executable(test_uri tests/test_uri.cc zdunk "${LIBS}")
zdunk_add_executable(test_application tests/test_application.cc zdunk "${LIBS}")

set(ORM_SRCS
    zdunk/orm/table.cc
    zdunk/orm/column.cc
    zdunk/orm/index.cc
    zdunk/orm/orm.cc
    zdunk/orm/util.cc
    )
# add_executable(orm ${ORM_SRCS})
# add_dependencies(orm zdunk)
# target_link_libraries(orm ${LIBS})
zdunk_add_executable(orm "${ORM_SRCS}" zdunk "${LIBS}")

add_library(test_module SHARED tests/test_module.cc)

zdunk_add_executable(bin_zdunk "zdunk/main.cc" zdunk "${LIBS}")
set_target_properties(bin_zdunk PROPERTIES OUTPUT_NAME "zdunk")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)