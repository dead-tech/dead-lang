cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)


project ("dead-lang" VERSION 0.1)

add_subdirectory ("src")
target_compile_options(dead-lang PRIVATE /WX /W4 /w44265 /w44061 /w44062 /permissive- /wd4100)
