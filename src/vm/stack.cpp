#include "stack.hpp"

namespace vm {
    [[nodiscard]] Object &Stack::top() noexcept
    {
        return data.top();
    }


    void Stack::pop() noexcept
    {
        data.pop();
    }

    void Stack::push(const Object &obj)
    {
        data.push(obj);
    }

    void Stack::push(Object &&obj)
    {
        data.push(obj);
    }
    
    std::size_t Stack::size() const noexcept
    {
        return data.size();
    }
}// namespace vm
