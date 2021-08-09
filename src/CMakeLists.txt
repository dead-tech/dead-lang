cmake_minimum_required (VERSION 3.10)
set(SRCS "main.cpp" "parser.cpp"  "vm/instructions.cpp"  "vm/vm.cpp" "vm/stack.cpp" "util/sv.cpp")

add_executable (dead-lang ${SRCS})

