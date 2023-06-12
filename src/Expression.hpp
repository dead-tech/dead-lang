#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Token.hpp"

class [[nodiscard]] Expression
{
  public:
    Expression() = default;

    virtual ~Expression() = default;

    Expression(const Expression& expression) = default;

    Expression(Expression&& expression) = default;

    Expression& operator=(const Expression& expression) = default;

    Expression& operator=(Expression&& expression) = default;

    [[nodiscard]] virtual std::string evaluate() const noexcept = 0;
};

class [[nodiscard]] UnaryExpression final : public Expression
{
  public:
    UnaryExpression(Token::Type unary_operator, std::shared_ptr<Expression> right) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    Token::Type                 m_operator;
    std::shared_ptr<Expression> m_right;
};

class [[nodiscard]] VariableExpression final : public Expression
{
  public:
    explicit VariableExpression(std::string variable_name) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string m_variable_name;
};

class [[nodiscard]] BinaryExpression final : public Expression
{
  public:
    BinaryExpression(
        std::shared_ptr<Expression> left,
        Token::Type                 binary_operator,
        std::shared_ptr<Expression> right) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_left;
    Token::Type                 m_operator;
    std::shared_ptr<Expression> m_right;
};

class [[nodiscard]] LiteralExpression final : public Expression
{
  public:
    explicit LiteralExpression(std::string literal) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string m_literal;
};

class [[nodiscard]] FunctionCallExpression final : public Expression
{
  public:
    FunctionCallExpression(std::string function_name, std::vector<std::shared_ptr<Expression>> arguments) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string                              m_function_name;
    std::vector<std::shared_ptr<Expression>> m_arguments;
};

class [[nodiscard]] IndexOperatorExpression final : public Expression
{
  public:
    IndexOperatorExpression(std::string variable_name, std::shared_ptr<Expression> index) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string                 m_variable_name;
    std::shared_ptr<Expression> m_index;
};


class [[nodiscard]] AssignmentExpression final : public Expression
{
  public:
    AssignmentExpression(
        std::shared_ptr<Expression> lhs,
        Token::Type                 assignment_operator,
        std::shared_ptr<Expression> rhs) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_lhs;
    Token::Type                 m_operator;
    std::shared_ptr<Expression> m_rhs;
};
