cmake_minimum_required(VERSION 3.15)
project(Utility)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

add_subdirectory(profiling_util)
add_subdirectory(math_util)
add_subdirectory(codec_util)

enable_testing()

add_test(hello_${PROJECT_NAME}_tests "echo" ${CMAKE_CURRENT_SOURCE_DIR})
