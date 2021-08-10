#ifndef PARSER_HPP
#define PARSER_HPP

#include <string_view>
#include <string>
#include <fstream>

#include "../util/sv.hpp"
#include "instructions.hpp"

namespace vm 
{
	std::vector<std::string> read_file(const char* file_path);
	std::string_view parse_op_code(const std::string_view line);
	std::optional<std::string_view> parse_string_literal(std::string_view line);
	instructions::Instruction parse_line(const std::string_view line) noexcept;
}

#endif //PARSER_HPP