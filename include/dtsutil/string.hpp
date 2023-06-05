#ifndef DTSUTIL_STRING_HPP
#define DTSUTIL_STRING_HPP

#include <string>
#include <string_view>
#include <vector>

namespace dts {

namespace detail {
template <typename T>
concept Stringy = requires (T str) {
    str.find_first_not_of('a', 0);
    str.substr(0, 0);
    str.find('a', 0);
};
}


[[nodiscard]] auto split_str(const detail::Stringy auto& str, const char delim) noexcept {
    std::vector<std::remove_cvref_t<decltype(str)>> splits = {};
    std::size_t end = 0;
    std::size_t start = str.find_first_not_of(delim, end);
    while (start != std::string::npos) {
        end = str.find(delim, start);
        splits.push_back(str.substr(start, end - start));
        start = str.find_first_not_of(delim, end);
    }

    return splits;
}

}

#endif //DTSUTIL_STRING_HPP
