#include "str.hpp"

namespace str {
    std::string ltrim(std::string sv)
    {
        sv.erase(sv.begin(), std::find_if(sv.begin(), sv.end(), [](unsigned char ch) {
                     return !std::isspace(ch);
                 }));
        return sv;
    }
    
    bool is_number(const std::string &str)
    {
        return str.find_first_not_of("0123456789") == std::string::npos;
    }
}// namespace str
