cmake_minimum_required(VERSION 3.20)
project(profiling)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main)
file(GLOB_RECURSE Main_Src "${SRC}/cpp/*.cpp")
add_library(${PROJECT_NAME} ${Main_Src})

target_include_directories(${PROJECT_NAME} PUBLIC ${SRC}/include)
