#pragma once

#include <expected>
#include <vector>
#include <memory>

#include <fmt/format.h>

#include "Token.hpp"
#include "Iterator.hpp"
#include "Position.hpp"
#include "Supervisor.hpp"

class [[nodiscard]] Lexer : public Iterator<std::string> {
public:
    [[nodiscard]] static std::vector<Token>
    lex(std::string source, const std::shared_ptr<Supervisor> &supervisor) noexcept;

private:
    explicit Lexer(std::string &&source, const std::shared_ptr<Supervisor> &supervisor) noexcept;

    [[nodiscard]] Token next_token() noexcept;

    void skip_whitespaces() noexcept;

    [[nodiscard]] Token lex_keyword_or_identifier() noexcept;

    [[nodiscard]] Token lex_minus() noexcept;

    [[nodiscard]] Token lex_equal_sign() noexcept;

    [[nodiscard]] Token lex_star() noexcept;

    [[nodiscard]] Token lex_plus() noexcept;

    [[nodiscard]] Token lex_less_than() noexcept;

    std::shared_ptr<Supervisor> m_supervisor;
};
