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

    [[nodiscard]] Token::Type operator_type() const noexcept
    {
        return m_operator;
    };

  private:
    Token::Type                 m_operator;
    std::shared_ptr<Expression> m_right;
};

class [[nodiscard]] VariableExpression final : public Expression
{
  public:
    explicit VariableExpression(std::string variable_name) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

    [[nodiscard]] constexpr std::string name() const noexcept
    {
        return m_variable_name;
    }

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

    [[nodiscard]] std::shared_ptr<Expression> left() const noexcept
    {
        return m_left;
    }

    [[nodiscard]] Token::Type operator_type() const noexcept
    {
        return m_operator;
    };

    [[nodiscard]] std::shared_ptr<Expression> right() const noexcept
    {
        return m_right;
    }

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
    FunctionCallExpression(
        std::shared_ptr<Expression>              function_name,
        std::vector<std::shared_ptr<Expression>> arguments) noexcept;

    [[nodiscard]] std::shared_ptr<Expression> function_name() const noexcept
    {
        return m_function_name;
    }

    [[nodiscard]] std::vector<std::shared_ptr<Expression>> arguments() const noexcept
    {
        return m_arguments;
    }

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression>              m_function_name;
    std::vector<std::shared_ptr<Expression>> m_arguments;
};

class [[nodiscard]] IndexOperatorExpression final : public Expression
{
  public:
    IndexOperatorExpression(std::shared_ptr<Expression> variable_name, std::shared_ptr<Expression> index) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_variable_name;
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


class [[nodiscard]] LogicalExpression final : public Expression
{
  public:
    LogicalExpression(
        std::shared_ptr<Expression> left,
        Token::Type                 logical_operator,
        std::shared_ptr<Expression> right) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_left;
    Token::Type                 m_operator;
    std::shared_ptr<Expression> m_right;
};


class [[nodiscard]] GroupingExpression final : public Expression
{
  public:
    explicit GroupingExpression(std::shared_ptr<Expression> expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_expression;
};


class [[nodiscard]] EnumExpression final : public Expression
{
  public:
    EnumExpression(std::shared_ptr<Expression> enum_base, std::shared_ptr<Expression> enum_variant) noexcept;

    [[nodiscard]] std::shared_ptr<Expression> enum_base() const noexcept
    {
        return m_enum_base;
    }

    [[nodiscard]] std::shared_ptr<Expression> enum_variant() const noexcept
    {
        return m_enum_variant;
    }

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_enum_base;
    std::shared_ptr<Expression> m_enum_variant;
};
