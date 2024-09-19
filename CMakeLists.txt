cmake_minimum_required(VERSION 3.29)
project(LetsConsole)

set(CMAKE_CXX_STANDARD 20)

add_executable(LetsConsole main.cpp
        ccolors.h
)

target_link_libraries(LetsConsole)
