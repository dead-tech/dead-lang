cmake_minimum_required(VERSION 3.10)
set(SRCS "main.cpp" "vm/parser.cpp" "vm/instructions.cpp" "vm/vm.cpp" "vm/stack.cpp" "util/sv.cpp" vm/exceptions.cpp vm/exceptions.hpp)

add_executable(dead-lang ${SRCS})
