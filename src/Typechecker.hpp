#pragma once

#include <cstdint>
#include <string>

class [[nodiscard]] Typechecker {
  public:
    enum class BuiltinType : std::uint8_t {
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

    [[nodiscard]] static constexpr BuiltinType builtin_type_from_string(const std::string& type) noexcept {
        if (type == "u8") {
            return BuiltinType::U8;
        } else if (type == "i8") {
            return BuiltinType::I8;
        } else if (type == "u16") {
            return BuiltinType::U16;
        } else if (type == "i16") {
            return BuiltinType::I16;
        } else if (type == "u32") {
            return BuiltinType::U32;
        } else if (type == "i32") {
            return BuiltinType::I32;
        } else if (type == "u64") {
            return BuiltinType::U64;
        } else if (type == "f32") {
            return BuiltinType::F32;
        } else if (type == "f64") {
            return BuiltinType::F64;
        } else if (type == "char") {
            return BuiltinType::CHAR;
        } else {
            return BuiltinType::NONE;
        }
    }

    [[nodiscard]] static constexpr std::string builtin_type_to_string(const BuiltinType& type) noexcept {
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

    [[nodiscard]] static constexpr std::string builtin_type_to_c_type(const BuiltinType& type) noexcept {
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

    [[nodiscard]] static constexpr std::string builtin_type_to_c_type(const std::string& type) noexcept {
        return builtin_type_to_c_type(builtin_type_from_string(type));
    }
};
