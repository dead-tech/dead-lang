#include "vm.hpp"

namespace vm
{
	void Vm::run(const char* file_path)
	{
		std::vector<std::string> code = read_file(file_path);

		while (true)
		{
			instructions::Instruction instruction = vm::parse_line(code[stack.ip]);
			instructions::map[instruction.op_code](stack, instruction);
		}
	}
};
