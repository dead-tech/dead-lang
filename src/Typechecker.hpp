#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
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
        NONE,
    };

    struct [[nodiscard]] CustomType
    {
        std::string name;
        Token::Type type;

        bool operator==(const CustomType& other) const noexcept
        {
            return name == other.name && type == other.type;
        }
    };

    struct [[nodiscard]] Type
    {
      public:
        constexpr explicit Type(CustomType type) : m_type{std::move(type)} {}

        constexpr explicit Type(BuiltinType type) : m_type{type} {}

        [[nodiscard]] constexpr std::variant<BuiltinType, CustomType> variant() const noexcept
        {
            return m_type;
        }

      private:
        std::variant<BuiltinType, CustomType> m_type;
    };

    struct [[nodiscard]] VariableDeclaration
    {
        bool        is_mutable;
        Type        type;
        std::string type_extensions;
        std::string name;
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
    is_valid_type(const std::string& token, const auto& custom_types) noexcept
    {
        const bool is_custom_type =
            std::ranges::find_if(custom_types, [&](const auto& map_entry) {
                const auto [custom_type, statement] = map_entry;
                return custom_type.name == token;
            }) != custom_types.end();

        return Typechecker::builtin_type_from_string(token) !=
                   Typechecker::BuiltinType::NONE ||
               is_custom_type;
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

    [[nodiscard]] static constexpr Type
    resolve_type(const std::string& type, const Token::Type& token_type) noexcept
    {
        return Typechecker::builtin_type_from_string(type) != Typechecker::BuiltinType::NONE
                 ? Type(Typechecker::builtin_type_from_string(type))
                 : Type(CustomType(type, token_type));
    }
};
