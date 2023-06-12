#include "Error.hpp"

DLError DLError::create(const std::string& message, const Position& position) noexcept
{
    return {message, position};
}

DLError::DLError(std::string message, const Position& position) noexcept
    : m_message{std::move(message)},
      m_position{position}
{
}

DLError::DLError(std::string message, const size_t start, const size_t end) noexcept
    : m_message{std::move(message)},
      m_position{Position::create(start, end)}
{
}
