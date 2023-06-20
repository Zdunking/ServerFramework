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
    )

add_library(zdunk SHARED ${LIB_SRC})
#add_library(zdunk_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(zdunk_static PROPERTIES OUTPUT_NAME "zdunk")

set(
    LIB_LIB
    zdunk
    pthread
    ${YAMLCPP}
)

add_executable(test tests/test.cc)
#add_dependencies(test zdunk)
#target_link_libraries(test zdunk yaml-cpp)
add_dependencies(test zdunk)
target_link_libraries(test ${LIB_LIB})

add_executable(test_config tests/test_config.cc)
#add_dependencies(test_config zdunk)
#target_link_libraries(test_config zdunk yaml-cpp)
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

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)