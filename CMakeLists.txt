cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} "-D_DEBUG")

project("dead-lang" VERSION 0.1)

add_subdirectory("src")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(dead-lang PRIVATE /WX /W4 /w44265 /w44061 /w44062 /permissive- /wd4100)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(dead-lang PRIVATE -Wall -Wextra -Werror -pedantic)
endif ()
