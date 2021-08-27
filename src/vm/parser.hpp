#ifndef PARSER_HPP
#define PARSER_HPP

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include "../util/sv.hpp"
#include "instructions.hpp"

namespace vm {
    std::vector<std::string> read_file(const std::filesystem::path &file_path);
    [[nodiscard]] std::vector<Label> parse_labels(const std::vector<std::string> &code);
    instructions::Instruction parse_line(std::string_view line) noexcept;
}// namespace vm

#endif//PARSER_HPP
