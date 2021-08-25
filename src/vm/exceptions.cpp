#include "exceptions.hpp"

namespace vm {

    VmError::VmError(const char *message) noexcept(true) : std::runtime_error(message), message(message)
    {
    }

    VmError::VmError(const char *message, std::size_t line_number) noexcept(true) : std::runtime_error(message)
    {
        this->message = "Line " + std::to_string(line_number) + " -> " + message;
    }

    [[nodiscard]] const char *VmError::what() const noexcept(true)
    {
        return message.c_str();
    }
}// namespace vm
