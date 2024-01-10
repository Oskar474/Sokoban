cmake_minimum_required(VERSION 3.26)
project(sokoban)

set(CMAKE_CXX_STANDARD 17)

add_executable(sokoban main.cpp)


find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(sokoban ${CURSES_LIBRARIES})