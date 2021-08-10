#include "parser.hpp"

namespace vm {

	std::vector<std::string> read_file(const char* file_path)
	{
		std::vector<std::string> code;

		//const auto path = std::filesystem::path(file_path);

		std::ifstream file(file_path);

		std::string line;
		while (std::getline(file, line))
		{
			code.push_back(line);
		}

		return code;
	}

	std::string_view parse_op_code(const std::string_view line)
	{
		const auto first_space = line.find_first_of(" ");
		return line.substr(0, first_space);
	}


	std::optional<std::string_view> parse_string_literal(std::string_view line)
	{
		const auto first_dquote = line.find_first_of("\"");
		const auto last_dquote = line.find_last_of("\"");


		if (first_dquote != std::string::npos && last_dquote != std::string::npos)
		{
			line.remove_suffix(1);
			return line.substr(first_dquote + 1, last_dquote - 1);
		}

		return std::nullopt;
	}

	instructions::Instruction parse_line(const std::string_view line) noexcept
	{
		constexpr std::string_view delim{ " " };

		const auto op_code = parse_op_code(line);
		const auto string_literal = parse_string_literal(std::string_view{ line });

		instructions::Arguments args;
		if (!string_literal)
		{
			const auto new_line = line.substr(op_code.size() + string_literal.value_or("").size(), line.size());
			args = sv::split_sv_to_int(new_line).value_or(instructions::Arguments());
		}

		return instructions::Instruction{ .op_code = op_code, .string_literal = string_literal, .args = args };

	}
};