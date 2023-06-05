#pragma once

#include <vector>

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "Error.hpp"

class [[nodiscard]] Supervisor {
  public:
    [[nodiscard]] static std::shared_ptr<Supervisor> create(std::string file_contents) noexcept;

    void push_error(const DLError& error) noexcept;

    template<typename... Args>
    void push_error(Args&&... args) noexcept {
        m_errors.push_back(DLError::create(std::forward<Args>(args)...));
    }

    void dump_errors() const;

    [[nodiscard]] constexpr bool has_errors() const noexcept { return !m_errors.empty(); }

  private:
    explicit Supervisor(std::string&& file_contents) noexcept;

    [[nodiscard]] std::vector<Position> compute_line_positions() const noexcept;

    void print_error(const DLError& error) const;

    mutable std::vector<DLError> m_errors;
    std::string                  m_file_contents;
};
