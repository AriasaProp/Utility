cmake_minimum_required(VERSION 3.20)
project(math)

# Menambahkan main source ke library
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main)
file(GLOB_RECURSE Main_Src "${SRC}/cpp/*.cpp" "${SRC}/c/*.c")
add_library(${PROJECT_NAME} STATIC ${Main_Src})
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC}/include)

# Menambahkan math_test
set(TSRC ${CMAKE_CURRENT_SOURCE_DIR}/test)
file(GLOB_RECURSE Test_Src "${TSRC}/cpp/*.cpp" "${TSRC}/c/*.c")
add_executable(${PROJECT_NAME}_test ${Test_Src})
target_include_directories(${PROJECT_NAME}_test PRIVATE ${TSRC}/include)

target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME} profiling)
