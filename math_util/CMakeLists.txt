cmake_minimum_required(VERSION 3.15)
project(math_util)

# Menambahkan main source ke library
set(LIBRARY_SRC_DIR ${CMAKE_SOURCE_DIR}/main)
file(GLOB_RECURSE Main_Src "${LIBRARY_SRC_DIR}/*.cpp" "${LIBRARY_SRC_DIR}/*.c")
add_library(${PROJECT_NAME} STATIC ${Main_Src})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBRARY_SRC_DIR})

# Menambahkan tests
set(TEST_SRC_DIR ${CMAKE_SOURCE_DIR}/test)
file(GLOB_RECURSE Test_Src "${TEST_SRC_DIR}/*.cpp" "${TEST_SRC_DIR}/*.c")
add_executable(tests ${Test_Src})
target_include_directories(tests PRIVATE 
		${TEST_SRC_DIR}
		../profiling_util/main
)

target_link_libraries(tests PRIVATE ${PROJECT_NAME} profiling_util)
set_target_properties(tests PROPERTIES LINKER_LANGUAGE CXX C)

add_test(run_tests
		COMMAND tests ${CMAKE_SOURCE_DIR}
    DEPENDS tests
)