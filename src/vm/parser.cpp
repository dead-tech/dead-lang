#include "parser.hpp"

namespace vm {

    static inline size_t line_number = 0;

    std::vector<std::string> read_file(const std::filesystem::path &file_path)
    {
        std::vector<std::string> code;

        if (!exists(file_path)) {
            std::cout << "Invalid path: " << file_path << '\n';
            exit(1);
        }

        std::ifstream file(file_path);

        std::string line;
        while (std::getline(file, line)) {
            code.push_back(line);
        }

        return code;
    }

    instructions::Instruction parse_line(const std::string_view line) noexcept
    {
        const auto split_line = sv::split_args(line);
        auto op_code = split_line[0];

        if (line.starts_with("//")) {
            op_code = "nop";
        }

        instructions::Arguments args;

        for (std::size_t i = 1; i < split_line.size(); ++i) {
            args.push_back(split_line[i]);
        }

        return instructions::Instruction{.op_code = op_code, .args = args, .line_number = ++line_number};
    }
}// namespace vm
