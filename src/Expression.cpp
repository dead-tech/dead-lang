#include "Expression.hpp"

UnaryExpression::UnaryExpression(Token::Type unary_operator, std::shared_ptr<Expression> right) noexcept
    : m_operator{unary_operator},
      m_right{std::move(right)}
{
}

std::string UnaryExpression::evaluate() const noexcept
{
    return fmt::format("{}{}", Token::type_to_string(m_operator), m_right->evaluate());
}

VariableExpression::VariableExpression(std::string variable_name) noexcept
    : m_variable_name{std::move(variable_name)}
{
}

std::string VariableExpression::evaluate() const noexcept
{
    return m_variable_name;
}

BinaryExpression::BinaryExpression(
    std::shared_ptr<Expression> left,
    Token::Type                 binary_operator,
    std::shared_ptr<Expression> right) noexcept
    : m_left{std::move(left)},
      m_operator{binary_operator},
      m_right{std::move(right)}
{
}

std::string BinaryExpression::evaluate() const noexcept
{
    switch (m_operator) {
        case Token::Type::COLON_COLON: {
            return fmt::format("{}::{}", m_left->evaluate(), m_right->evaluate());
        }
        case Token::Type::ARROW: {
            return fmt::format("{}->{}", m_left->evaluate(), m_right->evaluate());
        }
        case Token::Type::DOT: {
            return fmt::format("{}.{}", m_left->evaluate(), m_right->evaluate());
        }
        default: {
            return fmt::format(
                "{} {} {}",
                m_left->evaluate(),
                Token::type_to_string(m_operator),
                m_right->evaluate());
        }
    }
}

LiteralExpression::LiteralExpression(std::string literal) noexcept
    : m_literal{std::move(literal)}
{
}

std::string LiteralExpression::evaluate() const noexcept
{
    if (m_literal == "true") { return "1"; }
    if (m_literal == "false") { return "0"; }
    return m_literal;
}

FunctionCallExpression::FunctionCallExpression(
    std::shared_ptr<Expression>              function_name,
    std::vector<std::shared_ptr<Expression>> arguments) noexcept
    : m_function_name{std::move(function_name)},
      m_arguments{std::move(arguments)}
{
}

std::string FunctionCallExpression::evaluate() const noexcept
{
    std::string c_function_call_code = fmt::format("{}(", m_function_name->evaluate());
    for (const auto& argument : m_arguments) {
        c_function_call_code += argument->evaluate();
        if (&argument != &m_arguments.back()) { c_function_call_code += ", "; }
    }
    c_function_call_code += ")";
    return c_function_call_code;
}
IndexOperatorExpression::IndexOperatorExpression(
    std::shared_ptr<Expression> variable_name,
    std::shared_ptr<Expression> right) noexcept
    : m_variable_name{std::move(variable_name)},
      m_index{std::move(right)}
{
}

std::string IndexOperatorExpression::evaluate() const noexcept
{
    return fmt::format("{}[{}]", m_variable_name->evaluate(), m_index->evaluate());
}

AssignmentExpression::AssignmentExpression(
    std::shared_ptr<Expression> lhs,
    Token::Type                 assignment_operator,
    std::shared_ptr<Expression> rhs) noexcept
    : m_lhs{std::move(lhs)},
      m_operator{assignment_operator},
      m_rhs{std::move(rhs)}
{
}

std::string AssignmentExpression::evaluate() const noexcept
{
    return fmt::format(
        "{} {} {}", m_lhs->evaluate(), Token::type_to_string(m_operator), m_rhs->evaluate());
}

LogicalExpression::LogicalExpression(
    std::shared_ptr<Expression> left,
    Token::Type                 logical_operator,
    std::shared_ptr<Expression> right) noexcept
    : m_left{std::move(left)},
      m_operator{logical_operator},
      m_right{std::move(right)}
{
}

std::string LogicalExpression::evaluate() const noexcept
{
    const std::string logical_operator = [this] {
        switch (m_operator) {
            case Token::Type::AND:
                return "&&";
            case Token::Type::OR:
                return "||";
            default:
                return "";
        }
    }();

    return fmt::format("{} {} {}", m_left->evaluate(), logical_operator, m_right->evaluate());
}

GroupingExpression::GroupingExpression(std::shared_ptr<Expression> expression) noexcept
    : m_expression{std::move(expression)}
{
}

std::string GroupingExpression::evaluate() const noexcept
{
    return fmt::format("({})", m_expression->evaluate());
}

EnumExpression::EnumExpression(std::shared_ptr<Expression> enum_base, std::shared_ptr<Expression> enum_variant) noexcept
    : m_enum_base{std::move(enum_base)},
      m_enum_variant{std::move(enum_variant)}
{
}

std::string EnumExpression::evaluate() const noexcept
{
    return fmt::format("__dl_{}::{}", m_enum_base->evaluate(), m_enum_variant->evaluate());
}
