#include "Environment.hpp"

Environment::Environment(std::shared_ptr<Environment> parent) noexcept
    : m_parent{std::move(parent)}
{
}

void Environment::enscope(const Typechecker::VariableDeclaration& variable) noexcept
{
    m_variables.push_back(variable);
}

std::optional<Typechecker::VariableDeclaration>
Environment::find(const std::string& variable_name) const noexcept
{
    for (const auto& variable : m_variables) {
        if (variable.name == variable_name) { return variable; }
    }
    if (m_parent != nullptr) { return m_parent->find(variable_name); }
    return {};
}
