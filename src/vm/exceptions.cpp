#include "exceptions.hpp"

namespace vm {
    VmError::VmError() : std::runtime_error("")
    {
    }

    VmError::VmError(const char *message, std::size_t line_number) noexcept(true) : std::runtime_error(message)
    {
        this->message = "line " + std::to_string(line_number) + " -> " + message;
    }

    const char *VmError::what() const noexcept(true)
    {
        return message.c_str();
    }

    StackUnderflow::StackUnderflow(std::size_t line_number, std::size_t stack_size)
    {
        this->message = "Stack Underflow: Can't pop an element from the stack whose size is " + std::to_string(stack_size);
        error = VmError(this->message.c_str(), line_number);
    }

    const char *StackUnderflow::what() const noexcept(true)
    {
        return error.what();
    }

    VariableRedeclaration::VariableRedeclaration(std::size_t line_number, const std::string &var_name)
    {
        this->message = "Variable Redeclaration: Another variable named \"" + var_name + "\" already exists";
        error = VmError(this->message.c_str(), line_number);
    }

    const char *VariableRedeclaration::what() const noexcept(true)
    {
        return error.what();
    }

    UndeclaredVariable::UndeclaredVariable(std::size_t line_number, const std::string &var_name)
    {
        this->message = "Unknown Variable: No variable named \"" + var_name + "\" exists";
        error = VmError(this->message.c_str(), line_number);
    }

    const char *UndeclaredVariable::what() const noexcept(true)
    {
        return error.what();
    }
}// namespace vm
