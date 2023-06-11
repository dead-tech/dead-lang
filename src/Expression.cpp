#include "Expression.hpp"

UnaryExpression::UnaryExpression(Token::Type unary_operator, std::shared_ptr<Expression> right) noexcept
  : m_operator{ unary_operator },
    m_right{ std::move(right) } {}

std::string UnaryExpression::evaluate() const noexcept {
    return fmt::format("({}{})", Token::type_to_string(m_operator), m_right->evaluate());
}

VariableExpression::VariableExpression(std::string variable_name) noexcept
  : m_variable_name{ std::move(variable_name) } {}

std::string VariableExpression::evaluate() const noexcept { return m_variable_name; }

BinaryExpression::BinaryExpression(
  std::shared_ptr<Expression> left,
  Token::Type                 binary_operator,
  std::shared_ptr<Expression> right
) noexcept
  : m_left{ std::move(left) },
    m_operator{ binary_operator },
    m_right{ std::move(right) } {}

std::string BinaryExpression::evaluate() const noexcept {
    switch (m_operator) {
        case Token::Type::COLON_COLON: {
            return fmt::format("{}_{}", m_left->evaluate(), m_right->evaluate());
        }
        case Token::Type::ARROW: {
            return fmt::format("{}->{}", m_left->evaluate(), m_right->evaluate());
        }
        case Token::Type::DOT: {
            return fmt::format("{}.{}", m_left->evaluate(), m_right->evaluate());
        }
        default: {
            return fmt::format(
              "({} {} {})", m_left->evaluate(), Token::type_to_string(m_operator), m_right->evaluate()
            );
        }
    }
}

LiteralExpression::LiteralExpression(std::string literal) noexcept
  : m_literal{ std::move(literal) } {}

std::string LiteralExpression::evaluate() const noexcept { return m_literal; }

FunctionCallExpression::FunctionCallExpression(
  std::string                              function_name,
  std::vector<std::shared_ptr<Expression>> arguments
) noexcept
  : m_function_name{ std::move(function_name) },
    m_arguments{ std::move(arguments) } {}

std::string FunctionCallExpression::evaluate() const noexcept {
    std::string c_function_call_code = fmt::format("{}(", m_function_name);
    for (const auto& argument : m_arguments) {
        c_function_call_code += argument->evaluate();
        if (&argument != &m_arguments.back()) { c_function_call_code += ", "; }
    }
    c_function_call_code += ")";
    return c_function_call_code;
}