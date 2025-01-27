cmake_minimum_required(VERSION 3.15)
project(codec_util)

# Menambahkan main source ke library
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main)
file(GLOB_RECURSE Main_Src "${SRC}/*.cpp" "${SRC}/*.c")
add_library(${PROJECT_NAME} STATIC ${Main_Src})
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC})

# Menambahkan codec_tests
set(TEST_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
file(GLOB_RECURSE Test_Src "${TEST_SRC_DIR}/*.cpp" "${TEST_SRC_DIR}/*.c")
add_executable(${PROJECT_NAME}_tests ${Test_Src})
target_include_directories(${PROJECT_NAME}_tests PRIVATE ${TEST_SRC_DIR})

target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME} profiling_util)

add_test(run_${PROJECT_NAME}_tests ${PROJECT_NAME}_tests "${CMAKE_CURRENT_SOURCE_DIR}/data")