cmake_minimum_required(VERSION 3.10)
project(tetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/inc)

file(GLOB SOURCES "src/*.cpp")

add_executable(tetris ${SOURCES})

set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)
target_link_libraries(tetris PRIVATE ${CURSES_LIBRARIES} ncursesw)
