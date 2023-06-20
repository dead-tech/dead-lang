cmake_minimum_required(VERSION 3.25)
project(dead_lang)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic")

set(SOURCES src/main.cpp src/Lexer.cpp src/Parser.cpp src/Statement.cpp src/Typechecker.cpp src/Supervisor.cpp src/Error.cpp src/Position.cpp src/Token.cpp src/Expression.cpp src/Environment.cpp)
include_directories(include/)

add_executable(dead_lang ${SOURCES})
set_target_properties(dead_lang PROPERTIES OUTPUT_NAME "dl")
