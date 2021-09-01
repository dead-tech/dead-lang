#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "../util/sv.hpp"
#include "exceptions.hpp"
#include "stack.hpp"
#include <algorithm>
#include <any>
#include <array>
#include <concepts>
#include <functional>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

namespace vm {
    using VarMap = std::unordered_map<std::string, std::any>;
    using Label = std::vector<std::string>;

    template<typename Type>
    concept ValidType = requires
    {
        std::same_as<Type, std::string> || std::same_as<Type, int32_t>;
    };

    struct CallSite {
        std::string call_site_label;
        std::size_t offset_from_start;
    };

    struct VmState {
        Stack stack;
        VarMap vars;

        std::vector<Label> labels;
        std::string label_to_run = ".main";

        std::array<CallSite, 16> call_stack;
        std::size_t call_stack_ptr = 0;

        [[nodiscard]] Label get_label(const std::string &label_name, std::size_t line_number) const;
        template<ValidType Ret>
        auto get_variable(const std::string &label_name, std::size_t line_number) const -> std::tuple<decltype(vars.find(label_name)), Ret>;
        void set_variable(const std::string &label_name, const std::any &value, std::size_t line_number);
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

    void drop(VmState &state, const Instruction &instruction);

    void jump(VmState &state, const Instruction &instruction);

    void jump_if_not_equal(VmState &state, const Instruction &instruction);

    void ret(VmState &state, const Instruction &instruction);

    void dec(VmState &state, const Instruction &instruction);

    void inc(VmState &state, const Instruction &instruction);

    void add(VmState &state, const Instruction &instruction);

    void concat(VmState &state, const Instruction &instruction);

    void mov(VmState &state, const Instruction &instruction);

    void nop(VmState &state, const Instruction &instruction);

    void halt(VmState &state, const Instruction &instruction);

    inline std::unordered_map<std::string_view, FnPtr> map =
            {
                    {"push"sv, &push},
                    {"pop"sv, &pop},
                    {"swap"sv, &swap},
                    {"print"sv, &print},
                    {"set"sv, &set},
                    {"drop"sv, &drop},
                    {"jump"sv, &jump},
                    {"jumpne"sv, &jump_if_not_equal},
                    {"ret"sv, &ret},
                    {"dec"sv, &dec},
                    {"inc"sv, &inc},
                    {"add"sv, &add},
                    {"concat"sv, &concat},
                    {"mov"sv, &mov},
                    {"nop"sv, &nop},
                    {"halt"sv, &halt},
    };
}// namespace vm::instructions

namespace vm::instructions::impl {
    void print_var(VmState &state, [[maybe_unused]] const Instruction &instruction);

    void unconditional_jump(VmState &state, const std::string &label_name, std::size_t line_number);

    template<ValidType Type, typename BinaryOp>
    requires std::invocable<BinaryOp, Type &, Type &>
    void binary_op(VmState &state, const Instruction &instruction, size_t output, BinaryOp &&binary_operation);

    template<typename UnaryOp>
    requires std::invocable<UnaryOp, int32_t>
    void unary_op(VmState &state, [[maybe_unused]] const Instruction &instruction, UnaryOp &&unary_op);

    template<ValidType T>
    std::optional<T> get_v_opt(const std::any &any);
}// namespace vm::instructions::impl

#endif// INSTRUCTIONS_HPP
