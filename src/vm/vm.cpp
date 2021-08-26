#include "vm.hpp"

namespace vm {
    [[noreturn]] void Vm::run(const char *file_path)
    {
        const auto path = std::filesystem::path(file_path);
        std::vector<std::string> code = read_file(path);

        while (true) {
            instructions::Instruction instruction = vm::parse_line(code[state.stack.ip]);
            try {
                //Throw some error if no key exists
                instructions::map[instruction.op_code](state, instruction);
            }
            catch (const VmError &err) {
                std::cout << "Errors occurred while running, execution stopped.\n\n"
                          << "In file " << canonical(path) << " on " << err.what() << '\n';
                exit(1);
            }
        }
    }
}// namespace vm
