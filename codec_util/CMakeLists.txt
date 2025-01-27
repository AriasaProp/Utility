cmake_minimum_required(VERSION 3.15)
project(CodecUtil)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../profiling_util ../profiling_util/build)

enable_testing()

# Menambahkan main source ke library
set(LIBRARY_SRC_DIR ${CMAKE_SOURCE_DIR}/main)
file(GLOB_RECURSE Main_Src "${LIBRARY_SRC_DIR}/*.cpp" "${LIBRARY_SRC_DIR}/*.c")
add_library(codec_util STATIC ${Main_Src})
target_include_directories(codec_util PUBLIC ${LIBRARY_SRC_DIR})

# Menambahkan tests
set(TEST_SRC_DIR ${CMAKE_SOURCE_DIR}/test)
file(GLOB_RECURSE Test_Src "${TEST_SRC_DIR}/*.cpp" "${TEST_SRC_DIR}/*.c")
target_include_directories(tests PRIVATE
		${TEST_SRC_DIR}
		../profiling_util/main
)
add_executable(tests ${Test_Src})

target_link_libraries(tests PRIVATE codec_util profiling_util)

add_test(run_tests
		COMMAND tests ${CMAKE_SOURCE_DIR}/data
    DEPENDS tests
)