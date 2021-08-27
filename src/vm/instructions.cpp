#include "instructions.hpp"

namespace vm {
    std::optional<Label> VmState::get_label(const std::string &label_name) const noexcept
    {
        for (const auto &label : labels) {
            if (label[0].starts_with(label_name)) {
                return label;
            }
        }
        return std::nullopt;
    }
}// namespace vm

// ------------------------------------------------ INSTRUCTIONS ------------------------------------------------ //

namespace vm::instructions {

    void push(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw exceptions::VmError("Invalid arguments: `push` instruction requires 1 argument of type int or string", instruction.line_number);
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
            throw exceptions::StackUnderflow(instruction.line_number, stack_size);
        }

        state.stack.pop();
        state.stack.ip++;
    }

    void swap(VmState &state, [[maybe_unused]] const Instruction &instruction)
    {
        const auto stack_size = state.stack.size();

        if (stack_size < 2) {
            throw exceptions::SwapError(instruction.line_number, stack_size);
        }

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

        const auto stack_size = state.stack.size();

        if (stack_size < 1) {
            throw exceptions::StackUnderflow(instruction.line_number, stack_size);
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
            throw exceptions::VmError("Invalid Arguments: `set` instruction requires 2 arguments the variable name and its actual value", instruction.line_number);
        }

        const auto var_name = instruction.args[0];
        const auto var_value = instruction.args[1];

        if (!str::is_number(var_value)) {
            const auto [_, success] = state.vars.emplace(var_name, var_value);
            if (!success) {
                throw exceptions::VariableRedeclaration(instruction.line_number, var_name);
            }
        }
        else {
            const auto [_, success] = state.vars.emplace(var_name, std::atoi(var_value.c_str()));// NOLINT(cert-err34-c)
            if (!success) {
                throw exceptions::VariableRedeclaration(instruction.line_number, var_name);
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
            throw exceptions::UndeclaredVariable(instruction.line_number, instruction.args[0]);
        }

        const auto value = it->second;

        if (typeid(value) == typeid(int32_t)) {
            std::cout << std::any_cast<int32_t>(value) << '\n';
        }
        else {
            std::cout << std::any_cast<std::string>(value) << '\n';
        }
    }
}// namespace vm::instructions::impl
