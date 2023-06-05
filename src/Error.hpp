#pragma once

#include <string>
#include <utility>

#include "Position.hpp"

class [[nodiscard]] DLError {
  public:
    [[nodiscard]] static DLError create(const std::string& message, const Position& position) noexcept;

    template<typename... Args>
    [[nodiscard]] static DLError create(Args&&... args) noexcept {
        return { args... };
    }

    [[nodiscard]] std::string message() const noexcept { return m_message; }

    [[nodiscard]] Position position() const noexcept { return m_position; }

  private:
    DLError(std::string message, const Position& position) noexcept;

    DLError(std::string message, const size_t start, const size_t end) noexcept;

    std::string m_message;
    Position    m_position;
};
