#include "instructions.hpp"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning( disable: 4100 )
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

namespace vm::instructions
{

	void push_str(Stack& stack, const Instruction& instruction)
	{
		stack.push(Object{ .type = instruction.string_literal.value() });
		stack.ip++;
	}

	void push(Stack& stack, const Instruction& instruction)
	{
		if (instruction.args.size() > 0)
		{
			stack.push(Object{ .type = instruction.args[0] });
		}
		// ERROR
		// Throw a custom exception
		stack.ip++;
	}

	void pop(Stack& stack, const Instruction& instruction)
	{
		stack.pop();
		stack.ip++;
	}

	void swap(Stack& stack, const Instruction& instruction)
	{
		const Object a(std::move(stack.top()));
		stack.pop();
		const Object b(std::move(stack.top()));
		stack.pop();

		stack.push(a);
		stack.push(b);
		stack.ip++;
	}

	void print_str(Stack& stack, const Instruction& instruction)
	{
		std::cout << std::get<std::string_view>(stack.top().type) << '\n';
		stack.ip++;
	}

	void print(Stack& stack, const Instruction& instruction)
	{
		std::cout << std::get<int32_t>(stack.top().type) << '\n';
		stack.ip++;
	}

	void nop(Stack& stack, const Instruction& instruction)
	{
		stack.ip++;
	}

	void halt(Stack& stack, const Instruction& instruction)
	{
		stack.ip++;
		std::cout << "Shutdown!" << '\n';
		exit(0);
	}
}

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
