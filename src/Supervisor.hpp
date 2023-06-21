#pragma once

#include <filesystem>
#include <vector>

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "Error.hpp"

class [[nodiscard]] Supervisor
{
  public:
    [[nodiscard]] static std::shared_ptr<Supervisor>
    create(std::string file_contents, std::string project_root_file) noexcept;

    void push_error(const DLError& error) noexcept;

    template <typename... Args>
    void push_error(Args&&... args) noexcept
    {
        m_errors.push_back(DLError::create(std::forward<Args>(args)...));
    }

    void dump_errors() const;

    [[nodiscard]] constexpr bool has_errors() const noexcept
    {
        return !m_errors.empty();
    }

    [[nodiscard]] constexpr const std::filesystem::path& project_root() const noexcept
    {
        return m_project_root;
    }

  private:
    explicit Supervisor(std::string&& file_contents, std::filesystem::path project_root) noexcept;

    [[nodiscard]] std::vector<Position> compute_line_positions() const noexcept;

    void print_error(const DLError& error) const;

    mutable std::vector<DLError> m_errors;
    std::string                  m_file_contents;
    std::filesystem::path        m_project_root;
};
