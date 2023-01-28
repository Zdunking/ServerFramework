cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_COMPILER "/apps/stone/bin/gcc")
set(CMAKE_CXX_COMPILER "/apps/stone/bin/g++")

project(zdunk)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC
    zdunk/log.cc
    zdunk/utils.cc
    zdunk/config.cc
    )

add_library(zdunk SHARED ${LIB_SRC})
#add_library(zdunk_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(zdunk_static PROPERTIES OUTPUT_NAME "zdunk")

add_executable(test tests/test.cc)
add_dependencies(test zdunk)
target_link_libraries(test zdunk yaml-cpp)

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config zdunk)
target_link_libraries(test_config zdunk yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)