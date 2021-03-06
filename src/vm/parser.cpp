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

    std::vector<Label> parse_labels(const std::vector<std::string> &code)
    {
        std::vector<Label> out;

        // Throw if same label twice.
        for (auto it = code.begin(); it != code.end(); ++it) {
            if (it->starts_with(".") && it->ends_with("{")) {
                Label label;
                while (*it != "}") {
                    label.push_back(str::ltrim(*it));
                    ++it;
                }
                label.push_back(str::ltrim(*it));

                for (const auto &lb : out) {
                    if (lb[0] == label[0]) {
                        throw vm::exceptions::LabelRedeclaration(lb[0].substr(0, lb[0].size() - 2));
                    }
                }

                out.push_back(label);
            }
        }

        return out;
    }

    instructions::Instruction parse_line(const std::string_view line) noexcept
    {
        const auto split_line = sv::split_args(line);
        auto op_code = split_line[0];

        // Skips line if it is either a comment, a label declaration or the end of a label
        if (line.starts_with("//") ||
            line.starts_with(".") ||
            line == "}") {
            op_code = "nop";
        }

        instructions::Arguments args;

        for (std::size_t i = 1; i < split_line.size(); ++i) {
            args.push_back(split_line[i]);
        }

        return instructions::Instruction{.op_code = op_code, .args = args, .line_number = ++line_number};
    }
}// namespace vm
