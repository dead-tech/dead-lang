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
    using Label = std::vector<std::string>;
    struct VmState {
        Stack stack;
        VarMap vars;
        std::vector<Label> labels;

        [[nodiscard]] std::optional<Label> get_label(const std::string &label_name) const noexcept;
    };
}// namespace vm

namespace vm::instructions {
    using OpCode = std::string;
    using Arguments = std::vector<std::string>;

    struct Instruction {
        OpCode op_code;
        Arguments args;
        std::size_t line_number;
    };

    using FnPtr = void (*)(VmState &, const Instruction &);
    using namespace std::literals;

    void push(VmState &state, const Instruction &instruction);

    void pop(VmState &state, const Instruction &instruction);

    void swap(VmState &state, const Instruction &instruction);

    void print(VmState &state, const Instruction &instruction);

    void set(VmState &state, const Instruction &instruction);

    // Move this

    void nop(VmState &state, const Instruction &instruction);

    void halt(VmState &state, const Instruction &instruction);

    inline std::unordered_map<std::string_view, FnPtr> map =
            {
                    {"push"sv, &push},
                    {"pop"sv, &pop},
                    {"swap"sv, &swap},
                    {"print"sv, &print},
                    {"set"sv, &set},
                    {"nop"sv, &nop},
                    {"halt"sv, &halt},
    };
}// namespace vm::instructions

namespace vm::instructions::impl {
    void print_var(VmState &state, [[maybe_unused]] const Instruction &instruction);
}

#endif// INSTRUCTIONS_HPP
