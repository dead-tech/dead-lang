cmake_minimum_required(VERSION 3.25)
project(dead_lang)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic")

set(SOURCES src/main.cpp)
include_directories(include/)

add_executable(dead_lang ${SOURCES})
