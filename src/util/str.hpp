#ifndef STR_HPP
#define STR_HPP

#include <string>

namespace str {
    [[nodiscard]] std::string ltrim(std::string sv);
    [[nodiscard]] bool is_number(const std::string &str);
}// namespace str

#endif//STR_HPP
