#ifndef VM_HPP
#define VM_HPP

#include "parser.hpp"
#include <any>
#include <filesystem>

namespace vm {
    using Label = std::vector<std::string>;
    using VarMap = std::unordered_map<std::string, std::any>;

    class Vm {
    public:
        [[noreturn]] void run(const char *file_path);

    private:
        VmState state;
    };
}// namespace vm

#endif//VM_HPP
