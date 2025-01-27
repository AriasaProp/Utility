cmake_minimum_required(VERSION 3.15)
project(profiling_util)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main)
file(GLOB_RECURSE Main_Src "${SRC}/*.cpp" "${SRC}/*.c")
add_library(${PROJECT_NAME} ${Main_Src})

target_include_directories(${PROJECT_NAME} PUBLIC ${SRC})
