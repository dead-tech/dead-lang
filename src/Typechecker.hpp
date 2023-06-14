#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Expression.hpp"
#include "Token.hpp"

class [[nodiscard]] Typechecker
{
  public:
    enum class BuiltinType : std::uint8_t
    {
        U8 = 0,
        I8,
        U16,
        I16,
        U32,
        I32,
        U64,
        I64,
        F32,
        F64,
        CHAR,
        STRUCT,
        NONE,
    };

    struct [[nodiscard]] VariableDeclaration
    {
        BuiltinType                type;
        std::string                type_extensions;
        bool                       is_mutable;
        std::string                name;
        std::optional<std::string> custom_type;
    };

    [[nodiscard]] static constexpr BuiltinType builtin_type_from_string(const std::string& type) noexcept
    {
        if (type == "u8") { return BuiltinType::U8; }
        if (type == "i8") { return BuiltinType::I8; }
        if (type == "u16") { return BuiltinType::U16; }
        if (type == "i16") { return BuiltinType::I16; }
        if (type == "u32") { return BuiltinType::U32; }
        if (type == "i32") { return BuiltinType::I32; }
        if (type == "u64") { return BuiltinType::U64; }
        if (type == "f32") { return BuiltinType::F32; }
        if (type == "f64") { return BuiltinType::F64; }
        if (type == "char") { return BuiltinType::CHAR; }
        return BuiltinType::NONE;
    }

    [[nodiscard]] static constexpr std::string builtin_type_to_string(const BuiltinType& type) noexcept
    {
        switch (type) {
            case BuiltinType::U8: {
                return "u8";
            }
            case BuiltinType::I8: {
                return "i8";
            }
            case BuiltinType::U16: {
                return "u16";
            }
            case BuiltinType::I16: {
                return "i16";
            }
            case BuiltinType::U32: {
                return "u32";
            }
            case BuiltinType::I32: {
                return "i32";
            }
            case BuiltinType::U64: {
                return "u64";
            }
            case BuiltinType::I64: {
                return "i64";
            }
            case BuiltinType::F32: {
                return "f32";
            }
            case BuiltinType::F64: {
                return "f64";
            }
            case BuiltinType::CHAR: {
                return "char";
            }
            default: {
                return "unknown_builtin_type";
            }
        }
    }

    [[nodiscard]] static constexpr std::string builtin_type_to_c_type(const BuiltinType& type) noexcept
    {
        switch (type) {
            case BuiltinType::U8: {
                return "unsigned char";
            }
            case BuiltinType::I8: {
                return "char";
            }
            case BuiltinType::U16: {
                return "unsigned short";
            }
            case BuiltinType::I16: {
                return "short";
            }
            case BuiltinType::U32: {
                return "unsigned int";
            }
            case BuiltinType::I32: {
                return "int";
            }
            case BuiltinType::U64: {
                return "unsigned long";
            }
            case BuiltinType::I64: {
                return "long";
            }
            case BuiltinType::F32: {
                return "float";
            }
            case BuiltinType::F64: {
                return "double";
            }
            case BuiltinType::CHAR: {
                return "char";
            }
            default: {
                return "unknown_builtin_type";
            }
        }
    }

    [[nodiscard]] static constexpr std::string builtin_type_to_c_type(const std::string& type) noexcept
    {
        return builtin_type_to_c_type(builtin_type_from_string(type));
    }

    [[nodiscard]] static constexpr bool is_fixed_size_array(const std::string& type_extensions) noexcept
    {
        return !type_extensions.empty() && type_extensions.front() == '[' &&
               type_extensions.back() == ']';
    }

    [[nodiscard]] static constexpr bool
    is_valid_type(const Token& token, const std::vector<std::string>& custom_types) noexcept
    {
        if (!token.matches(Token::Type::IDENTIFIER)) { return false; }

        return Typechecker::builtin_type_from_string(token.lexeme()) !=
                   Typechecker::BuiltinType::NONE ||
               std::ranges::find(custom_types, token.lexeme()) != custom_types.end();
    }

    [[nodiscard]] static bool is_valid_lvalue(const std::shared_ptr<Expression>& expression) noexcept
    {
        if (const auto unary_expression =
                std::dynamic_pointer_cast<UnaryExpression>(expression)) {
            return unary_expression->operator_type() == Token::Type::STAR;
        }

        if (const auto binary_expression =
                std::dynamic_pointer_cast<BinaryExpression>(expression)) {
            return binary_expression->operator_type() == Token::Type::DOT ||
                   binary_expression->operator_type() == Token::Type::ARROW;
        }

        return std::dynamic_pointer_cast<VariableExpression>(expression) ||
               std::dynamic_pointer_cast<IndexOperatorExpression>(expression);
    }
};
