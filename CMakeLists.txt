# CMakeLists.txt
cmake_minimum_required(VERSION 3.17)   

project("yate") 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB MY_SOURCES "src/*.cpp" "src/*.hpp" "src/*.h")
add_executable("${PROJECT_NAME}" ${MY_SOURCES})

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})