#include "instructions.hpp"

namespace vm::instructions {

    void push(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw VmError("Invalid arguments: `push` instruction requires 1 argument of type int or string", instruction.line_number);
        }

        if (!str::is_number(instruction.args[0])) {
            state.stack.push(Object{.value = instruction.args[0]});
        }
        else {
            state.stack.push(Object{.value = std::atoi(instruction.args[0].c_str())});// NOLINT(cert-err34-c)
        }

        state.stack.ip++;
    }

    void pop(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {

        const std::size_t stack_size = state.stack.size();

        if (stack_size < 1) {
            throw StackUnderflow(instruction.line_number, stack_size);
        }

        state.stack.pop();
        state.stack.ip++;
    }

    void swap(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        const Object a(state.stack.top());
        state.stack.pop();
        const Object b(state.stack.top());
        state.stack.pop();

        state.stack.push(a);
        state.stack.push(b);
        state.stack.ip++;
    }

    void print(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        if (!instruction.args.empty()) {
            impl::print_var(state, instruction);
            state.stack.ip++;
            return;
        }

        const auto top = state.stack.top().value;

        if (std::holds_alternative<int32_t>(top)) {
            std::cout << std::get<int32_t>(top) << '\n';
        }
        else {
            std::cout << std::get<std::string>(top) << '\n';
        }

        state.stack.ip++;
    }

    void set(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        if (instruction.args.size() < 2) {
            throw VmError("Invalid Arguments: `set` instruction requires 2 arguments the variable name and its actual value", instruction.line_number);
        }

        if (!str::is_number(instruction.args[1])) {
            const auto [_, success] = state.vars.emplace(instruction.args[0], instruction.args[1]);
            if (!success) {
                throw VariableRedeclaration(instruction.line_number, instruction.args[0]);
            }
        }
        else {
            const int32_t to_insert = std::atoi(instruction.args[1].c_str());// NOLINT(cert-err34-c)
            const auto [_, success] = state.vars.emplace(instruction.args[0], to_insert);
            if (!success) {
                throw VariableRedeclaration(instruction.line_number, instruction.args[0]);
            }
        }

        state.stack.ip++;
    }

    void nop(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        state.stack.ip++;
    }

    void halt(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        state.stack.ip++;
        exit(0);
    }
}// namespace vm::instructions

namespace vm::instructions::impl {
    void print_var(VmState &state, const Instruction &instruction)
    {
        const auto it = state.vars.find(instruction.args[0]);
        
        if (it == state.vars.end()) {
            throw UndeclaredVariable(instruction.line_number, instruction.args[0]);
        }

        if (it->second.type() == typeid(int32_t)) {
            std::cout << std::any_cast<int32_t>(it->second) << '\n';
        }
        else {
            std::cout << std::any_cast<std::string>(it->second) << '\n';
        }
    }
}// namespace vm::instructions::impl
