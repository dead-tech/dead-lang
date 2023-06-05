#pragma once

#include <expected>
#include <memory>
#include <vector>

#include <fmt/format.h>

#include "Iterator.hpp"
#include "Position.hpp"
#include "Supervisor.hpp"
#include "Token.hpp"

class [[nodiscard]] Lexer : public Iterator<std::string> {
  public:
    [[nodiscard]] static std::vector<Token>
      lex(std::string source, const std::shared_ptr<Supervisor>& supervisor) noexcept;

  private:
    explicit Lexer(std::string&& source, const std::shared_ptr<Supervisor>& supervisor) noexcept;

    [[nodiscard]] Token next_token() noexcept;

    void skip_whitespaces() noexcept;

    [[nodiscard]] Token lex_keyword_or_identifier() noexcept;

    [[nodiscard]] Token lex_minus() noexcept;

    [[nodiscard]] Token lex_equal_sign() noexcept;

    [[nodiscard]] Token lex_star() noexcept;

    [[nodiscard]] Token lex_plus() noexcept;

    [[nodiscard]] Token lex_less_than() noexcept;

    [[nodiscard]] Token lex_single_quoted_string() noexcept;

    [[nodiscard]] Token lex_number() noexcept;

    std::shared_ptr<Supervisor> m_supervisor;
};
