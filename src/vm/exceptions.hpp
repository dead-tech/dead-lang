#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace vm {
    class VmError : public std::runtime_error {
    public:
        explicit VmError(const char *message) noexcept(true);
        VmError(const char *message, std::size_t line_number) noexcept(true);
        [[nodiscard]] const char *what() const noexcept(true) override;

    private:
        std::string message;
    };
}// namespace vm

#endif//EXCEPTIONS_HPP
