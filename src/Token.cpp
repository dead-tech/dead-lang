#include "Token.hpp"

[[nodiscard]] Token Token::create(Type type, std::string lexeme, Position position) noexcept {
    return Token{type, std::move(lexeme), position};
}

[[nodiscard]] Token Token::create_dumb() noexcept {
    return Token::create(Type::END_OF_FILE, "", Position::create_dumb());
}

Token::Token(Token::Type type, std::string lexeme, Position position) noexcept
        : m_type{type}, m_lexeme{std::move(lexeme)}, m_position(position) {}
