#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <iostream>
#include <unordered_map>
#include <optional>
#include <vector>
#include "stack.hpp"

namespace vm::instructions 
{
	using OpCode = std::string_view;
	using StringLiteral = std::optional<std::string_view>;
	using Arguments = std::vector<int32_t>;

	struct Instruction
	{
		std::string_view op_code;
		std::optional<std::string_view> string_literal;
		std::vector<int32_t> args;
	};

	using FnPtr = void (*)(Stack&, const Instruction&);
	using namespace std::literals;

	void push_str(Stack& stack, const Instruction& instruction);

	void push(Stack& stack, const Instruction& instruction);

	void pop(Stack& stack, const Instruction& instruction);

	void swap(Stack& stack, const Instruction& instruction);

	void print_str(Stack& stack, const Instruction& instruction);

	void print(Stack& stack, const Instruction& instruction);

	void nop(Stack& stack, const Instruction& instruction);

	void halt(Stack& stack, const Instruction& instruction);

	inline std::unordered_map<std::string_view, FnPtr> map =
	{
		{"pushstr"sv, &push_str},
		{"push"sv, &push},
		{"pop"sv, &pop},
		{"swap"sv, &swap},
		{"printstr"sv, &print_str},
		{"print"sv, &print},
		{"nop"sv, &nop},
		{"halt"sv, &halt},
	};
}

#endif // INSTRUCTIONS_HPP