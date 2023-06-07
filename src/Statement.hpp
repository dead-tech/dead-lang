#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <dtsutil/string.hpp>

#include "Typechecker.hpp"

class [[nodiscard]] Statement {
  public:
    Statement() = default;

    virtual ~Statement() = default;

    Statement(const Statement&) = default;

    Statement(Statement&&) = default;

    Statement& operator=(const Statement&) = default;

    Statement& operator=(Statement&&) = default;

    [[nodiscard]] virtual std::string evaluate() const noexcept = 0;
};

class [[nodiscard]] EmptyStatement final : public Statement {
  public:
    EmptyStatement() noexcept = default;

    [[nodiscard]] std::string evaluate() const noexcept override;
};

class [[nodiscard]] BlockStatement final : public Statement {
  public:
    explicit BlockStatement(std::vector<std::shared_ptr<Statement>> block) noexcept;

    [[nodiscard]] auto empty() const noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

    void append(const std::shared_ptr<Statement>& statement) noexcept;

  private:
    std::vector<std::shared_ptr<Statement>> m_block;
};

class [[nodiscard]] ModuleStatement final : public Statement {
  public:
    explicit ModuleStatement(std::string name) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

    [[nodiscard]] auto& c_includes() noexcept { return m_c_includes; }

    [[nodiscard]] const auto& c_includes() const noexcept { return m_c_includes; }

    [[nodiscard]] auto& structs() noexcept { return m_structs; }

    [[nodiscard]] auto& functions() noexcept { return m_functions; }

    [[nodiscard]] const auto& structs() const noexcept { return m_structs; }

    [[nodiscard]] const auto& functions() const noexcept { return m_functions; }

  private:
    std::string              m_name;
    std::vector<std::string> m_c_includes;
    BlockStatement           m_structs;
    BlockStatement           m_functions;
};

class [[nodiscard]] FunctionStatement final : public Statement {
  public:
    FunctionStatement(std::string name, std::string args, std::string return_type, BlockStatement body) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string    m_name;
    std::string    m_args;
    std::string    m_return_type;
    BlockStatement m_body;
};

class [[nodiscard]] IfStatement final : public Statement {
  public:
    IfStatement(std::string condition, BlockStatement then_block, BlockStatement else_block) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string    m_condition;
    BlockStatement m_then_block;
    BlockStatement m_else_block;
};

class [[nodiscard]] ReturnStatement final : public Statement {
  public:
    explicit ReturnStatement(std::string expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string m_expression;
};

class [[nodiscard]] VariableStatement final : public Statement {
  public:
    VariableStatement(
      const bool               is_mutable,
      Typechecker::BuiltinType type,
      std::string              type_extensions,
      std::string              name,
      std::string              expression
    ) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    bool                     m_is_mutable;
    Typechecker::BuiltinType m_type;
    std::string              m_type_extensions;
    std::string              m_name;
    std::string              m_expression;
};

class [[nodiscard]] PlusEqualStatement final : public Statement {
  public:
    PlusEqualStatement(std::string name, std::string expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string m_name;
    std::string m_expression;
};

class [[nodiscard]] WhileStatement final : public Statement {
  public:
    WhileStatement(std::string condition, BlockStatement body) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string    m_condition;
    BlockStatement m_body;
};

class [[nodiscard]] ForStatement final : public Statement {
  public:
    ForStatement(
      std::shared_ptr<Statement> init_statement,
      std::string                condition,
      std::string                increment_statement,
      BlockStatement             body
    ) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Statement> m_init_statement;
    std::string                m_condition;
    std::string                m_increment_statement;
    BlockStatement             m_body;
};

class [[nodiscard]] ExpressionStatement final : public Statement {
  public:
    explicit ExpressionStatement(std::string expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string m_expression;
};

class [[nodiscard]] ArrayStatement final : public Statement {
  public:
    ArrayStatement(
      bool                     is_mutable,
      Typechecker::BuiltinType type,
      std::string              type_extensions,
      std::string              name,
      std::string              elements
    ) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    bool                     m_is_mutable;
    Typechecker::BuiltinType m_type;
    std::string              m_type_extensions;
    std::string              m_name;
    std::string              m_elements;
};

class [[nodiscard]] IndexOperatorStatement final : public Statement {
  public:
    IndexOperatorStatement(std::string variable_name, std::string index, std::string expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string m_variable_name;
    std::string m_index;
    std::string m_expression;
};


class [[nodiscard]] FunctionCallStatement final : public Statement {
  public:
    FunctionCallStatement(std::string name, std::string args) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string m_name;
    std::string m_args;
};

class [[nodiscard]] StructStatement final : public Statement {
  public:
    StructStatement(std::string name, std::vector<std::string> member_variables) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string              m_name;
    std::vector<std::string> m_member_variables;
};
