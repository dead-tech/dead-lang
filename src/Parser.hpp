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

    [[nodiscard]] std::shared_ptr<Statement>
      parse_variable_statement(const Token::Type& ending_delimiter = Token::Type::END_OF_LINE) noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_variable_assignment() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_plus_equal_statement(const std::string&& variable_name) noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_while_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_for_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_expression_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_array_statement(
      const bool                      is_mutable,
      const Typechecker::BuiltinType& array_type,
      const std::string&              type_extensions
    ) noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_index_operator_statement(const std::string&& variable_name) noexcept;

    [[nodiscard]] std::string parse_c_include_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_function_call_statement() noexcept;

    [[nodiscard]] std::shared_ptr<Statement> parse_struct_statement() noexcept;

    [[nodiscard]] std::string parse_expression(const Token::Type& delimiter) noexcept;

    [[nodiscard]] std::vector<std::shared_ptr<Statement>> parse_statement_block() noexcept;

    [[nodiscard]] std::string parse_identifier() noexcept;

    [[nodiscard]] std::vector<std::string> parse_member_variables() noexcept;

    [[nodiscard]] Position previous_position() const noexcept;

    template<std::invocable Callable>
    void consume_tokens_until(const Token::Type& delimiter, Callable&& callable) noexcept;

    [[nodiscard]] bool matches_and_consume(const Token::Type& delimiter) noexcept;

    [[nodiscard]] bool eol() const noexcept;

    std::shared_ptr<Supervisor> m_supervisor;
};
