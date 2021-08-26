#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace vm {
    class VmError : public std::runtime_error {
    public:
        explicit VmError();
        VmError(const char *message, std::size_t line_number) noexcept(true);
        [[nodiscard]] const char *what() const noexcept(true) override;

    protected:
        std::string message;
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

}// namespace vm

#endif//EXCEPTIONS_HPP
