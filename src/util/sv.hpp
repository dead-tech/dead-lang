#ifndef SV_HPP
#define SV_HPP

#include <cctype>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace sv {
    [[nodiscard]] std::vector<std::string> split_args(std::string_view sv);
    [[nodiscard]] std::string ltrim(std::string sv);

}// namespace sv

#endif//SV_HPP
