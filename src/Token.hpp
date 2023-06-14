#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>

#include <fmt/format.h>

#include "Position.hpp"

class [[nodiscard]] Token
{
  public:
    enum class Type : std::uint8_t
    {
        // Single-character tokens
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        COMMA,
        DOT,
        MINUS,
        PLUS,
        SEMICOLON,
        SLASH,
        STAR,
        AMPERSAND,
        COLON,

        // Multi-character tokens
        COLON_COLON,

        // Comparison
        BANG,
        BANG_EQUAL,
        EQUAL,
        EQUAL_EQUAL,
        GREATER,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL,

        // Arithmetics
        MINUS_MINUS,
        PLUS_PLUS,
        PLUS_EQUAL,

        // Others
        ARROW,


        // Keywords
        AND,
        CLASS,
        ELSE,
        TRUE,
        FALSE,
        FN,
        FOR,
        IF,
        OR,
        RETURN,
        WHILE,
        MUT,
        C_INCLUDE,
        STRUCT,

        // Literals
        IDENTIFIER,
        SINGLE_QUOTED_STRING,
        DOUBLE_QUOTED_STRING,
        NUMBER,

        // Magic tokens
        END_OF_LINE,
        END_OF_FILE,
        MAX,
    };

    [[nodiscard]] static Token create(Type type, std::string lexeme, Position position) noexcept;

    [[nodiscard]] static Token create_dumb() noexcept;

    [[nodiscard]] constexpr Type type() const noexcept { return m_type; }

    [[nodiscard]] constexpr std::string lexeme() const noexcept
    {
        return m_lexeme;
    }

    [[nodiscard]] Position position() const noexcept { return m_position; }

    [[nodiscard]] constexpr bool matches(const Type& rhs_type) const noexcept
    {
        return m_type == rhs_type;
    }

    [[nodiscard]] constexpr static std::optional<Type> is_keyword(const std::string& lexeme) noexcept
    {
        if (lexeme == "fn") { return Type::FN; }
        if (lexeme == "if") { return Type::IF; }
        if (lexeme == "mut") { return Type::MUT; }
        if (lexeme == "return") { return Type::RETURN; }
        if (lexeme == "while") { return Type::WHILE; }
        if (lexeme == "for") { return Type::FOR; }
        if (lexeme == "include") { return Type::C_INCLUDE; }
        if (lexeme == "struct") { return Type::STRUCT; }
        if (lexeme == "true") { return Type::TRUE; }
        if (lexeme == "false") { return Type::FALSE; }
        if (lexeme == "class") { return Type::CLASS; }
        return {};
    }

    [[nodiscard]] constexpr static bool is_equality_operator(const Token& token) noexcept
    {
        constexpr std::array<Type, 2> comparison_operators = {
            Type::EQUAL_EQUAL,
            Type::BANG_EQUAL,
        };

        return std::ranges::find(comparison_operators, token.type()) !=
               comparison_operators.end();
    }

    [[nodiscard]] constexpr static bool is_comparison_operator(const Token& token) noexcept
    {
        constexpr std::array<Type, 4> comparison_operators = {
            Type::GREATER,
            Type::GREATER_EQUAL,
            Type::LESS,
            Type::LESS_EQUAL,
        };

        return std::ranges::find(comparison_operators, token.type()) !=
               comparison_operators.end();
    }

    [[nodiscard]] constexpr static bool is_assignment_operator(const Token& token) noexcept
    {
        constexpr std::array<Type, 2> assignment_operators = {
            Type::EQUAL,
            Type::PLUS_EQUAL,
        };

        return std::ranges::find(assignment_operators, token.type()) !=
               assignment_operators.end();
    }

    [[nodiscard]] constexpr static bool is_literal(const Token& token) noexcept
    {
        constexpr std::array<Type, 3> literals = {
            Type::NUMBER,
            Type::SINGLE_QUOTED_STRING,
            Type::DOUBLE_QUOTED_STRING,
        };

        return std::ranges::find(literals, token.type()) != literals.end();
    }

    [[nodiscard]] constexpr static bool is_unary_operator(const Token& token) noexcept
    {
        constexpr std::array<Type, 5> unary_operators = {
            Type::MINUS,
            Type::BANG,
            Type::PLUS_PLUS,
            Type::AMPERSAND,
            Type::STAR,
        };

        return std::ranges::find(unary_operators, token.type()) !=
               unary_operators.end();
    }

    [[nodiscard]] constexpr static bool is_boolean(const Token& token) noexcept
    {
        return token.type() == Type::TRUE || token.type() == Type::FALSE;
    }

    [[nodiscard]] constexpr static std::string type_to_string(const Type& type) noexcept
    {
        static_assert(static_cast<std::uint8_t>(Type::MAX) == 48, "Exhaustive handling of all Token::Type enum variants is required."); // NOLINT

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
            case Type::FOR: {
                return "for";
            }
            case Type::LEFT_BRACKET: {
                return "[";
            }
            case Type::RIGHT_BRACKET: {
                return "]";
            }
            case Type::END_OF_LINE: {
                return "eol";
            }
            case Type::C_INCLUDE: {
                return "include";
            }
            case Type::SINGLE_QUOTED_STRING: {
                return "single quoted string";
            }
            case Type::DOUBLE_QUOTED_STRING: {
                return "double quoted string";
            }
            case Type::STRUCT: {
                return "struct";
            }
            case Type::COLON: {
                return ":";
            }
            case Type::COLON_COLON: {
                return "::";
            }
            case Type::PLUS: {
                return "+";
            }
            case Type::LESS: {
                return "<";
            }
            case Type::PLUS_PLUS: {
                return "++";
            }
            case Type::AMPERSAND: {
                return "&";
            }
            case Type::SLASH: {
                return "/";
            }
            case Type::BANG: {
                return "!";
            }
            case Type::BANG_EQUAL: {
                return "!=";
            }
            case Type::GREATER: {
                return ">";
            }
            case Type::GREATER_EQUAL: {
                return ">=";
            }
            case Type::TRUE: {
                return "true";
            }
            case Type::FALSE: {
                return "false";
            }
            case Type::CLASS: {
                return "class";
            }
            default: {
                return "not implemented";
            }
        }
    }

  private:
    Token(Type type, std::string lexeme, Position position) noexcept;

    Type        m_type;
    std::string m_lexeme;
    Position    m_position;
};

// {fmt} formatters
template <>
struct fmt::formatter<Token::Type>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Token::Type& type, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", Token::type_to_string(type));
    }
};

template <>
struct fmt::formatter<Token>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Token& token, FormatContext& ctx)
    {
        return fmt::format_to(
            ctx.out(),
            "{{ TokenType: {}, Lexeme: {}, Position: {} }}",
            token.type(),
            token.lexeme(),
            token.position());
    }
};
