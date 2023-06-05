#pragma once

#include <string>
#include <cstdint>

class [[nodiscard]] Typechecker {
public:
    enum class BuiltinType : std::uint8_t {
        I32 = 0,
        NONE,
    };

    [[nodiscard]] static constexpr BuiltinType builtin_type_from_string(const std::string& type) noexcept {
        if (type == "i32") {
            return BuiltinType::I32;
        } else {
            return BuiltinType::NONE;
        }
    }

    [[nodiscard]] static constexpr std::string builtin_type_to_string(const BuiltinType& type) noexcept {
        switch (type) {
            case BuiltinType::I32: {
                return "i32";
            }
            default: {
                return "unknown_builtin_type";
            }
        }
    }

    [[nodiscard]] static constexpr std::string builtin_type_to_c_type(const BuiltinType& type) noexcept {
        switch (type) {
            case BuiltinType::I32: {
                return "int";
            }
            default: {
                return "unknown_builtin_type";
            }
        }
    }

    [[nodiscard]] static constexpr std::string builtin_type_to_c_type(const std::string& type) noexcept {
        return builtin_type_to_c_type(builtin_type_from_string(type));
    }
};
