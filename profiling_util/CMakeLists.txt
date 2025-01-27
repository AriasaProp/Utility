cmake_minimum_required(VERSION 3.15)
project(profiling_util)

file(GLOB_RECURSE Main_Src "${CMAKE_SOURCE_DIR}/main/*.cpp" "${CMAKE_SOURCE_DIR}/main/*.c")
add_library(${PROJECT_NAME} ${Main_Src})

target_include_directories(${PROJECT_NAME} PUBLIC {CMAKE_SOURCE_DIR}/main)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX C)
