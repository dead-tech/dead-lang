#include "exceptions.hpp"

namespace vm::exceptions {
    VmError::VmError() : std::runtime_error("")
    {
    }

    VmError::VmError(const char *message, const std::size_t line_number) noexcept(true) : std::runtime_error(message)
    {
        this->message = "line " + std::to_string(line_number) + " -> " + message;
    }

    const char *VmError::what() const noexcept(true)
    {
        return message.c_str();
    }

    UnknownOpCode::UnknownOpCode(const std::size_t line_number, const std::string_view op_code)
    {
        this->message = "Unknown OpCode: No opcode named \"" + std::string{op_code} + "\" exists";
        error = VmError(this->message.c_str(), line_number);
    }

    const char *UnknownOpCode::what() const noexcept(true)
    {
        return error.what();
    }

    SwapError::SwapError(std::size_t line_number, const std::size_t stack_size)
    {
        this->message = "Swap Error: In order to swap stack size must be >= 2 but stack size was " + std::to_string(stack_size);
        error = VmError(this->message.c_str(), line_number);
    }

    const char *SwapError::what() const noexcept(true)
    {
        return error.what();
    }

    StackUnderflow::StackUnderflow(const std::size_t line_number, const std::size_t stack_size)
    {
        this->message = "Stack Underflow: Can't access/pop an element from the stack whose size is " + std::to_string(stack_size);
        error = VmError(this->message.c_str(), line_number);
    }

    const char *StackUnderflow::what() const noexcept(true)
    {
        return error.what();
    }

    VariableRedeclaration::VariableRedeclaration(const std::size_t line_number, const std::string &var_name)
    {
        this->message = "Variable Redeclaration: Another variable named \"" + var_name + "\" already exists";
        error = VmError(this->message.c_str(), line_number);
    }

    const char *VariableRedeclaration::what() const noexcept(true)
    {
        return error.what();
    }

    UndeclaredVariable::UndeclaredVariable(const std::size_t line_number, const std::string &var_name)
    {
        this->message = "Unknown Variable: No variable named \"" + var_name + "\" exists";
        error = VmError(this->message.c_str(), line_number);
    }

    const char *UndeclaredVariable::what() const noexcept(true)
    {
        return error.what();
    }

    UndeclaredLabel::UndeclaredLabel(std::size_t line_number, const std::string &label_name)
    {
        this->message = "(Line number may be incorrect) Unknown Label: No label named \"" + label_name + "\" exists";
        error = VmError(this->message.c_str(), line_number);
    }

    const char *UndeclaredLabel::what() const noexcept(true)
    {
        return error.what();
    }

    NonReturningLabel::NonReturningLabel(std::size_t line_number, const std::string &label_name)
    {
        this->message = "Non-Returning Label: No `ret` instruction in label \"" + label_name + "\" was found";
        error = VmError(this->message.c_str(), line_number);
    }

    const char *NonReturningLabel::what() const noexcept(true)
    {
        return error.what();
    }

    LabelRedeclaration::LabelRedeclaration(const std::string &label_name)
    {
        this->message = "(Line number is not correct) Label Redeclaration: Another label named \"" + label_name + "\" already exists";
        error = VmError(this->message.c_str(), 0);
    }

    const char *LabelRedeclaration::what() const noexcept(true)
    {
        return error.what();
    }

    CallStackUnderflow::CallStackUnderflow(std::size_t line_number, std::size_t stack_size)
    {
        this->message = "CallStack Underflow: Cant' return from function because call stack size is " + std::to_string(stack_size);
        error = VmError(this->message.c_str(), line_number);
    }

    const char *CallStackUnderflow::what() const noexcept(true)
    {
        return error.what();
    }
}// namespace vm::exceptions
