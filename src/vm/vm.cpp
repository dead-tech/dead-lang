#include "vm.hpp"

namespace vm {
    [[noreturn]] void Vm::run(const char *file_path)
    {
        std::vector<std::string> code = read_file(file_path);

        while (true) {
            instructions::Instruction instruction = vm::parse_line(code[state.stack.ip]);
            try {
                instructions::map[instruction.op_code](state, instruction);
            }
            catch (const VmError &err) {
                std::cout << "Errors occurred while running, execution stopped.\n"
                          << err.what() << '\n';
                std::cout << "Process terminated with exit code 1" << '\n';
                exit(1);
            }
        }
    }
}// namespace vm
