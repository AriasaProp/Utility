cmake_minimum_required(VERSION 3.15)
project(ProfilingUtil)

file(GLOB_RECURSE Main_Src "${CMAKE_SOURCE_DIR}/main/*.cpp" "${CMAKE_SOURCE_DIR}/main/*.c")
add_library(profiling_util ${Main_Src})

target_include_directories(profiling_util PUBLIC {CMAKE_SOURCE_DIR}/main)
