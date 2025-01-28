cmake_minimum_required(VERSION 3.20)
project(Utility)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(profiling_util)
add_subdirectory(math_util)
add_subdirectory(codec_util)

include(CTest)
