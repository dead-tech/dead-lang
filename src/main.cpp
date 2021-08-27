#include "vm/vm.hpp"

void usage() noexcept
{
    std::cout << "usage: dead-lang <file>" << '\n';
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        usage();
        exit(1);
    }

    vm::Vm vm;
    
    try {
        vm.run(argv[1]);
    }
    catch (const vm::exceptions::VmError &err) {
        std::cout << "Errors occurred while running, execution stopped.\n\n"
                  << "In file " << canonical(std::filesystem::path{argv[1]}) << " on " << err.what() << '\n';
        exit(1);
    }
}
