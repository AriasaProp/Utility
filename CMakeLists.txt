cmake_minimum_required(VERSION 3.20)
project(Utility)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_BINARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

# set library
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main)
file(GLOB_RECURSE Main_Src
		"${SRC}/src/*.cpp"
		"${SRC}/src/*.c")
add_library(${PROJECT_NAME} STATIC ${Main_Src})
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC}/include)

add_definitions(-DMATH_TEST)

# set test executable
set(TSRC ${CMAKE_CURRENT_SOURCE_DIR}/test)
file(GLOB_RECURSE Test_Src
	"${TSRC}/src/*.cpp"
	"${TSRC}/src/*.c")
add_executable(${PROJECT_NAME}_test ${Test_Src})
target_include_directories(${PROJECT_NAME}_test PRIVATE ${TSRC}/include)
target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})

#Cxxdroid-binary: build/Utility_test