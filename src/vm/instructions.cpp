#include "instructions.hpp"

namespace vm::instructions {

    void push_str(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw VmError("Invalid arguments: `pushstr` instruction requires 1 argument of value string", instruction.line_number);
        }

        state.stack.push(Object{.value = instruction.args[0]});
        state.stack.ip++;
    }

    void push(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw VmError("Invalid arguments: `push` instruction requires 1 argument of value int", instruction.line_number);
        }

        state.stack.push(Object{.value = std::atoi(instruction.args[0].c_str())});// NOLINT(cert-err34-c)
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

    void print_str(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        std::cout << std::get<std::string>(state.stack.top().value) << '\n';
        state.stack.ip++;
    }

    void print(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        std::cout << std::get<int32_t>(state.stack.top().value) << '\n';
        state.stack.ip++;
    }

    void set(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        if (instruction.args.size() < 2) {
            throw VmError("Invalid Arguments: `set` instruction requires 2 arguments the variable name and its actual value", instruction.line_number);
        }

        const auto is_number = [](const std::string &str) -> bool {
            return str.find_first_not_of("0123456789") == std::string::npos;
        };

        if (!is_number(instruction.args[1])) {
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

    void print_var(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw VmError("Invalid Arguments: `printvar` instruction requires 1 argument the variable name", instruction.line_number);
        }

        // Crash if no such key exists.
        if (const auto it = state.vars.find(instruction.args[0]); it == state.vars.end()) {
            throw UndeclaredVariable(instruction.line_number, instruction.args[0]);
        }

        const auto value = state.vars[instruction.args[0]];

        if (value.type() == typeid(int32_t)) {
            std::cout << std::any_cast<int32_t>(value) << '\n';
        }
        else {
            std::cout << std::any_cast<std::string>(value) << '\n';
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
        std::cout << "\nProcess terminated with exit code 0!" << '\n';
        exit(0);
    }
}// namespace vm::instructions
