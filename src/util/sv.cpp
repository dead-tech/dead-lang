#include "sv.hpp"

namespace sv {

    std::string ltrim(std::string sv)
    {
        sv.erase(sv.begin(), std::find_if(sv.begin(), sv.end(), [](unsigned char ch) {
                     return !std::isspace(ch);
                 }));
        return sv;
    }

    std::vector<std::string> split_args(std::string_view sv)
    {
        std::vector<std::string> out;

        auto it = sv.begin();
        std::size_t offset = 0;
        std::size_t spaces = 0;

        while (it != sv.end()) {
            if (std::isspace(*it)) {
                const auto idx = std::distance(sv.begin(), it);
                const std::string to_insert = ltrim(std::string{sv.substr(std::distance(sv.begin(), sv.begin() + offset), idx - offset)});
                out.push_back(to_insert);
                offset = idx;
                ++spaces;
            }
            else if (*it == '"') {
                const auto idx = std::distance(sv.begin(), it);
                const auto closing_dquote_pos = sv.find_last_of('"');

                std::advance(it, closing_dquote_pos - std::distance(sv.begin(), it));

                std::string_view to_insert = sv.substr(idx + 1, closing_dquote_pos);
                to_insert.remove_suffix(1);
                out.emplace_back(to_insert);
            }
            ++it;
        }

        if (out.size() != spaces + 1) {
            const std::string to_insert = ltrim(std::string{sv.substr(std::distance(sv.begin(), sv.begin() + offset), sv.size() - offset)});
            out.push_back(to_insert);
        }

        return out;
    }
}// namespace sv
