#include "vm/vm.hpp"

void usage() noexcept
{
	std::cout << "usage: dead-lang <file>" << '\n';
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		usage();
		exit(1);
	}

	vm::Vm vm;
	vm.run(argv[1]);

	std::cout << "VM didn't shutdown correctly. Try using halt instruction!" << '\n';
	return 1;
}
