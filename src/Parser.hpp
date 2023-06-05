#pragma once

#include <algorithm>
#include <concepts>
#include <memory>
#include <optional>
#include <vector>

#define FMT_HEADER_ONLY

#include <fmt/core.h>

#include "Iterator.hpp"
#include "Statement.hpp"
#include "Supervisor.hpp"
#include "Token.hpp"
#include "Typechecker.hpp"

class [[nodiscard]] Parser : public Iterator<std::vector<Token>> {
  public:
    [[nodiscard]] static std::shared_ptr<Statement>
      parse(std::vector<Token> tokens, const std::shared_ptr<Supervisor>& supervisor) noexcept;

  private:
    explicit Parser(std::vector<Token>&& tokens, const std::shared_ptr<Supervisor>& supervisor) noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_module() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_function_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_if_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_return_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_variable_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_variable_assignment() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_plus_equal_statement(const std::string& variable_name) noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_while_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_for_statement() noexcept;

    [[nodiscard]] std::string parse_expression(const Token::Type& delimiter) noexcept;

    [[nodiscard]] std::vector<std::shared_ptr<Statement>> parse_statement_block() noexcept;

    [[nodiscard]] Position previous_position() const noexcept;

    template<std::invocable Callable>
    void consume_tokens_until(const Token::Type& delimiter, Callable&& callable) noexcept;

    [[nodiscard]] bool matches_and_consume(const Token::Type& delimiter) noexcept;

    std::shared_ptr<Supervisor> m_supervisor;
};
