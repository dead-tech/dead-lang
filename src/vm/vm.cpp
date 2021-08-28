#include "vm.hpp"

namespace vm {
    [[noreturn]] void Vm::run(const char *file_path)
    {
        const auto path = std::filesystem::path(file_path);
        std::vector<std::string> code = read_file(path);

        state.labels = vm::parse_labels(code);

        while (true) {

            const Label label = state.get_label(state.label_to_run, 0);
            instructions::Instruction instruction = vm::parse_line(label[state.stack.ip]);

            const auto found = instructions::map.contains(instruction.op_code);

            if (!found) {
                throw exceptions::UnknownOpCode(instruction.line_number, instruction.op_code);
            }

            instructions::map[instruction.op_code](state, instruction);
        }
    }
}// namespace vm
