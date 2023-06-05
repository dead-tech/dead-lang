#pragma once

#include <cstdint>
#include <string>
#include <sstream>

#include <fmt/format.h>

#include "Position.hpp"

class [[nodiscard]] Token {
public:
    enum class Type : std::uint8_t {
        // Single-character tokens
        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
        COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

        // Multi-character tokens
        // Comparison
        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL,
        GREATER, GREATER_EQUAL,
        LESS, LESS_EQUAL,

        // Arithmetics
        MINUS_MINUS, PLUS_EQUAL,

        // Others
        ARROW,


        // Keywords
        AND, CLASS, ELSE, FALSE, FN, FOR, IF, OR,
        RETURN, TRUE, WHILE, MUT,

        // Literals
        IDENTIFIER, STRING, NUMBER,

        // Magic tokens
        END_OF_FILE
    };

    [[nodiscard]] static Token create(Type type, std::string lexeme, Position position) noexcept;
    [[nodiscard]] static Token create_dumb() noexcept;

    [[nodiscard]] constexpr Type type() const noexcept { return m_type; }
    [[nodiscard]] constexpr std::string lexeme() const noexcept { return m_lexeme; }
    [[nodiscard]] Position position() const noexcept { return m_position; }

    [[nodiscard]] constexpr bool matches(const Type &rhs_type) const noexcept { return m_type == rhs_type; }

    [[nodiscard]] constexpr static Type is_keyword(const std::string &lexeme) noexcept {
        if (lexeme == "fn") {
            return Type::FN;
        } else if (lexeme == "if") {
            return Type::IF;
        } else if (lexeme == "mut") {
            return Type::MUT;
        } else if (lexeme == "return") {
            return Type::RETURN;
        } else if (lexeme == "while") {
            return Type::WHILE;
        } else {
            return Type::END_OF_FILE;
        }
    }


    [[nodiscard]] constexpr static std::string type_to_string(const Type& type) noexcept {
        switch (type) {
            case Type::FN: {
                return "fn";
            }
            case Type::LEFT_PAREN: {
                return "(";
            }
            case Type::RIGHT_PAREN: {
                return ")";
            }
            case Type::MINUS: {
                return "-";
            }
            case Type::LEFT_BRACE: {
                return "{";
            }
            case Type::RIGHT_BRACE: {
                return "}";
            }
            case Type::EQUAL: {
                return "=";
            }
            case Type::EQUAL_EQUAL: {
                return "==";
            }
            case Type::ARROW: {
                return "->";
            }
            case Type::MINUS_MINUS: {
                return "--";
            }
            case Type::SEMICOLON: {
                return ";";
            }
            case Type::STAR: {
                return "*";
            }
            case Type::IDENTIFIER: {
                return "identifier";
            }
            case Type::END_OF_FILE: {
                return "eof";
            }
            case Type::COMMA: {
                return ",";
            }
            case Type::MUT: {
                return "mut";
            }
            case Type::PLUS_EQUAL: {
                return "+=";
            }
            case Type::WHILE: {
                return "while";
            }
            default: {
                return "not implemented";
            }
        }
    }

private:
    Token(Type type, std::string lexeme, Position position) noexcept;

    Type m_type;
    std::string m_lexeme;
    Position m_position;
};

// {fmt} formatters
template <>
struct fmt::formatter<Token::Type> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Token::Type& type, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", Token::type_to_string(type));
    }
};

template <>
struct fmt::formatter<Token> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Token& token, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{{ TokenType: {}, Lexeme: {}, Position: {} }}", token.type(), token.lexeme(), token.position());
    }
};
