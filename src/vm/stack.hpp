#ifndef STACK_HPP
#define STACK_HPP

#include <stack>
#include <string>
#include <string_view>
#include <variant>

namespace vm {

    struct Object {
    private:
        using value_type = std::variant<int32_t, uint32_t, std::string>;

    public:
        value_type value;
    };

    struct Stack {
    public:
        [[nodiscard]] Object &top() noexcept;

        void pop() noexcept;

        void push(const Object &obj);

        void push(Object &&obj);

        [[nodiscard]] std::size_t size() const noexcept;

    public:
        Stack()
            : ip{0}, data{}
        {
        }

        std::size_t ip;

    private:
        std::stack<Object> data;
    };
}// namespace vm
#endif// STACK_HPP
