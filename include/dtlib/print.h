#ifndef DTLIB_PRINT_H
#define DTLIB_PRINT_H

#include <concepts>
#include <iostream>
#include <sstream>
#include <string_view>

#include "config.h"

namespace dt {
    template<typename... Args>
    concept Streamable = requires(std::ostream &stream, Args &&...args)
    {
        { (stream << ... << args) } -> std::convertible_to<std::ostream &>;
    };

    DTLIB_NODISCARD("Useless call to function")
    std::string format(const std::string_view fmt) noexcept
    {
        return std::string{fmt};
    }

    // This function could be constexpr when std::stringstream constexpr ctor is implemented in GCC/Clang
    template<typename T, typename... Args>
    requires Streamable<Args...>
    DTLIB_NODISCARD("Useless call to function")
    auto format(const std::string_view fmt, T to_print, Args &&...args) noexcept
    {
        std::stringstream out{};
        const auto placeholder = fmt.find('%');
        if (placeholder != std::string::npos) {
            out << fmt.substr(0, placeholder);
            out << to_print;
            out << format(fmt.substr(placeholder + 1, fmt.size()), args...);
        }
        else {
            out << fmt;
        }
        return out.str();
    }

    template<typename T, typename... Args>
    requires Streamable<Args...>
    DTLIB_CONSTEXPR void print(const std::string_view fmt, T to_print, Args &&...args) noexcept
    {
        std::cout << format(fmt, to_print, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    requires Streamable<Args...>
    DTLIB_CONSTEXPR void println(const std::string_view fmt, T to_print, Args &&...args) noexcept
    {
        std::cout << format(fmt, to_print, std::forward<Args>(args)...) << '\n';
    }

    void puts(const char *msg) noexcept
    {
        std::puts(msg);
    }

}// namespace dt

#endif//DTLIB_PRINT_H
