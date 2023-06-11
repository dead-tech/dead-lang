#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <dtsutil/string.hpp>

#include "Expression.hpp"
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

    [[nodiscard]] const std::vector<std::shared_ptr<Statement>>& data() const noexcept { return m_block; }

    [[nodiscard]] std::string evaluate() const noexcept override;

    void append(const std::shared_ptr<Statement>& statement) noexcept;

  private:
    std::vector<std::shared_ptr<Statement>> m_block;
};

class [[nodiscard]] ModuleStatement final : public Statement {
  public:
    explicit ModuleStatement(
      std::string              name,
      std::vector<std::string> c_includes,
      BlockStatement           structs,
      BlockStatement           functions
    ) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string              m_name;
    std::vector<std::string> m_c_includes;
    BlockStatement           m_structs;
    BlockStatement           m_functions;
};

class [[nodiscard]] FunctionStatement final : public Statement {
  public:
    FunctionStatement(
      std::string                                   name,
      std::vector<Typechecker::VariableDeclaration> args,
      std::string                                   return_type,
      BlockStatement                                body
    ) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string                                   m_name;
    std::vector<Typechecker::VariableDeclaration> m_args;
    std::string                                   m_return_type;
    BlockStatement                                m_body;
};

class [[nodiscard]] IfStatement final : public Statement {
  public:
    IfStatement(std::shared_ptr<Expression> condition, BlockStatement then_block, BlockStatement else_block) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_condition;
    BlockStatement              m_then_block;
    BlockStatement              m_else_block;
};

class [[nodiscard]] ReturnStatement final : public Statement {
  public:
    explicit ReturnStatement(std::shared_ptr<Expression> expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_expression;
};

class [[nodiscard]] VariableStatement final : public Statement {
  public:
    VariableStatement(Typechecker::VariableDeclaration variable, std::shared_ptr<Expression> expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    Typechecker::VariableDeclaration m_variable_declaration;
    std::shared_ptr<Expression>      m_expression;
};

class [[nodiscard]] PlusEqualStatement final : public Statement {
  public:
    PlusEqualStatement(std::string name, std::shared_ptr<Expression> expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string                 m_name;
    std::shared_ptr<Expression> m_expression;
};

class [[nodiscard]] WhileStatement final : public Statement {
  public:
    WhileStatement(std::shared_ptr<Expression> condition, BlockStatement body) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_condition;
    BlockStatement              m_body;
};

class [[nodiscard]] ForStatement final : public Statement {
  public:
    ForStatement(
      std::shared_ptr<Statement>  init_statement,
      std::shared_ptr<Expression> condition,
      std::shared_ptr<Expression> increment_statement,
      BlockStatement              body
    ) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Statement>  m_init_statement;
    std::shared_ptr<Expression> m_condition;
    std::shared_ptr<Expression> m_increment_statement;
    BlockStatement              m_body;
};

class [[nodiscard]] ExpressionStatement final : public Statement {
  public:
    explicit ExpressionStatement(std::shared_ptr<Expression> expression) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::shared_ptr<Expression> m_expression;
};

class [[nodiscard]] ArrayStatement final : public Statement {
  public:
    ArrayStatement(
      Typechecker::VariableDeclaration         variable_declaration,
      std::vector<std::shared_ptr<Expression>> elements
    ) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    Typechecker::VariableDeclaration         m_variable_declaration;
    std::vector<std::shared_ptr<Expression>> m_elements;
};

class [[nodiscard]] IndexOperatorStatement final : public Statement {
  public:
    IndexOperatorStatement(
      std::string                 variable_name,
      std::shared_ptr<Expression> index,
      std::shared_ptr<Expression> expression
    ) noexcept;

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string                 m_variable_name;
    std::shared_ptr<Expression> m_index;
    std::shared_ptr<Expression> m_expression;
};

class [[nodiscard]] StructStatement final : public Statement {
  public:
    StructStatement(std::string name, std::vector<Typechecker::VariableDeclaration> member_variables) noexcept;

    [[nodiscard]] const std::string& name() const noexcept { return m_name; }

    [[nodiscard]] std::string evaluate() const noexcept override;

  private:
    std::string                                   m_name;
    std::vector<Typechecker::VariableDeclaration> m_member_variables;
};
