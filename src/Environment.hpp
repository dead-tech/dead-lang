#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Typechecker.hpp"

class [[nodiscard]] Environment
{
  public:
    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> parent) noexcept;

    [[nodiscard]] std::shared_ptr<Environment> parent() const noexcept { return m_parent; }

    void enscope(const Typechecker::VariableDeclaration& variable) noexcept;

    [[nodiscard]] std::optional<Typechecker::VariableDeclaration>
    find(const std::string& variable_name) const noexcept;

  private:
    std::vector<Typechecker::VariableDeclaration> m_variables;
    std::shared_ptr<Environment>                  m_parent = nullptr;
};
