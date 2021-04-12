# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)   

project("yate") 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_GENERATOR "Unix Makefiles")
if (WIN32) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

file(GLOB MY_SOURCES "src/*.cpp" "src/*.hpp" "src/*.h")
add_executable("${PROJECT_NAME}" ${MY_SOURCES})

if (WIN32)
	add_definitions(-DYATE_WINDOWS)
	set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
	find_package(PDCurses REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE PDCurses)
else()
	find_package(Curses REQUIRED Curses)
	target_link_libraries(${PROJECT_NAME} PRIVATE Curses)
endif()
