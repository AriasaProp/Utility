cmake_minimum_required(VERSION 3.15)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(profiling_util profiling_util/build)
add_subdirectory(math_util math_util/build)
add_subdirectory(codec_util codec_util/build)

enable_testing()

