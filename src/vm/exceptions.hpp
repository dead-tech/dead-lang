#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <filesystem>
#include <stdexcept>
#include <string>

namespace vm::exceptions {
    class VmError : public std::runtime_error {
    public:
        explicit VmError();
        VmError(const char *message, std::size_t line_number) noexcept(true);
        [[nodiscard]] const char *what() const noexcept(true) override;

    protected:
        std::string message;
    };

    class UnknownOpCode : public VmError {
    public:
        UnknownOpCode(std::size_t line_number, std::string_view op_code);
        [[nodiscard]] const char *what() const noexcept(true) final;

    private:
        VmError error;
    };

    class SwapError : public VmError {
    public:
        SwapError(std::size_t line_number, const std::size_t stack_size);
        [[nodiscard]] const char *what() const noexcept(true) final;

    private:
        VmError error;
    };

    class StackUnderflow : public VmError {
    public:
        StackUnderflow(std::size_t line_number, std::size_t stack_size);
        [[nodiscard]] const char *what() const noexcept(true) final;

    private:
        VmError error;
    };

    class VariableRedeclaration : public VmError {
    public:
        VariableRedeclaration(std::size_t line_number, const std::string &var_name);
        [[nodiscard]] const char *what() const noexcept(true) final;

    private:
        VmError error;
    };

    class UndeclaredVariable : public VmError {
    public:
        UndeclaredVariable(std::size_t line_number, const std::string &var_name);
        [[nodiscard]] const char *what() const noexcept(true) final;

    private:
        VmError error;
    };

}// namespace vm::exceptions

#endif//EXCEPTIONS_HPP
