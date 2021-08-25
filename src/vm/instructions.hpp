#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "../util/sv.hpp"
#include "exceptions.hpp"
#include "stack.hpp"
#include <algorithm>
#include <any>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

namespace vm {
    using VarMap = std::unordered_map<std::string, std::any>;
    struct VmState {
        Stack stack;
        VarMap vars;
    };
}// namespace vm

namespace vm::instructions {
    using OpCode = std::string_view;
    using Arguments = std::vector<std::string>;

    struct Instruction {
        OpCode op_code;
        Arguments args;
        std::size_t line_number;
    };

    using FnPtr = void (*)(VmState &, const Instruction &);
    using namespace std::literals;

    void push_str(VmState &state, const Instruction &instruction);

    void push(VmState &state, const Instruction &instruction);

    void pop(VmState &state, const Instruction &instruction);

    void swap(VmState &state, const Instruction &instruction);

    void print_str(VmState &state, const Instruction &instruction);

    void print(VmState &state, const Instruction &instruction);

    void set(VmState &state, const Instruction &instruction);

    void print_var(VmState &state, [[maybe_unused]] const Instruction &instruction);

    void nop(VmState &state, const Instruction &instruction);

    void halt(VmState &state, const Instruction &instruction);

    inline std::unordered_map<std::string_view, FnPtr> map =
            {
                    {"pushstr"sv, &push_str},
                    {"push"sv, &push},
                    {"pop"sv, &pop},
                    {"swap"sv, &swap},
                    {"printstr"sv, &print_str},
                    {"print"sv, &print},
                    {"set"sv, &set},
                    {"printvar"sv, &print_var},
                    {"nop"sv, &nop},
                    {"halt"sv, &halt},
    };
}// namespace vm::instructions

#endif// INSTRUCTIONS_HPP
