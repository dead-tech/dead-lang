#ifndef STACK_HPP
#define STACK_HPP

#include <variant>
#include <stack>
#include <string_view>

namespace vm
{

	struct Object
	{
	private:
		using value_type = std::variant<int32_t, uint32_t, std::string_view>;

	public:
		value_type type;
	};

	struct Stack
	{
	public:
		[[nodiscard]] Object& top() noexcept;

		void pop() noexcept;

		void push(const Object& obj);

		void push(Object&& obj);

	public:
		Stack()
			: ip{ 0 }, data{}
		{}

		std::size_t ip;
	private:
		std::stack<Object> data;
	};
};
#endif// STACK_HPP