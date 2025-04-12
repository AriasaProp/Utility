cmake_minimum_required(VERSION 3.20)
project(Utility)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_definitions(-DMINING_TEST)

# set library
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main)
file(GLOB_RECURSE Main_Src
		"${SRC}/src/*.cpp"
		"${SRC}/src/*.c")
add_library(${PROJECT_NAME} STATIC ${Main_Src})
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC}/include)

# set test executable
set(TSRC ${CMAKE_CURRENT_SOURCE_DIR}/test)
file(GLOB_RECURSE Test_Src
	"${TSRC}/src/*.cpp"
	"${TSRC}/src/*.c")
add_executable(${PROJECT_NAME}_test ${Test_Src})
target_include_directories(${PROJECT_NAME}_test PRIVATE ${TSRC}/include)
target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})


find_package(SDL2 REQUIRED)

# set sdl executable
set(SDLSRC ${CMAKE_CURRENT_SOURCE_DIR}/sdl)
file(GLOB_RECURSE SDL_Src
	"${SDLSRC}/src/*.cpp"
	"${SDLSRC}/src/*.c")
add_executable(${PROJECT_NAME}_sdl ${SDL_Src})
target_include_directories(${PROJECT_NAME}_sdl PRIVATE ${SDLSRC}/include)
target_link_libraries(${PROJECT_NAME}_sdl PRIVATE ${PROJECT_NAME} ${SDL_LIBRARIES})

#Cxxdroid: Utility_sdl