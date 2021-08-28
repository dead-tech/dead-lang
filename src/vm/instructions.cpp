#include "instructions.hpp"

namespace vm {
    Label VmState::get_label(const std::string &label_name, const std::size_t line_number) const
    {
        for (const auto &label : labels) {
            if (label[0].starts_with(label_name)) {
                return label;
            }
        }

        throw exceptions::UndeclaredLabel(line_number, label_name);
    }

    template<typename Ret>
    auto VmState::get_variable(const std::string &label_name, const std::size_t line_number) const -> std::tuple<decltype(vars.find(label_name)), Ret>
    {
        auto it = vars.find(label_name);

        if (it == vars.end()) {
            throw exceptions::UndeclaredVariable(line_number, label_name);
        }

        const std::optional<Ret> integer = instructions::impl::get_v_opt<Ret>(it->second);
        return std::tuple{it, integer.value()};
    }

    void VmState::set_variable(const std::string &label_name, const std::any &value, const std::size_t line_number)
    {
        auto it = vars.find(label_name);

        if (it == vars.end()) {
            throw exceptions::UndeclaredVariable(line_number, label_name);
        }

        it->second = value;
    }
}// namespace vm

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

    void pop(VmState &state, const Instruction &instruction)
    {

        const std::size_t stack_size = state.stack.size();

        if (stack_size < 1) {
            throw exceptions::StackUnderflow(instruction.line_number, stack_size);
        }

        state.stack.pop();
        state.stack.ip++;
    }

    void swap(VmState &state, const Instruction &instruction)
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

    void print(VmState &state, const Instruction &instruction)
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

    void set(VmState &state, const Instruction &instruction)
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

    void drop(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw exceptions::VmError("Invalid Arguments: `drop` instruction requires 1 argument the variable name", instruction.line_number);
        }

        const auto node_handle = state.vars.extract(instruction.args[0]);

        if (node_handle.empty())
        {
            throw exceptions::UndeclaredVariable(instruction.line_number, instruction.args[0]);
        }

        state.stack.ip++;
    }

    void jump(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw exceptions::VmError("Invalid Arguments: `jump` instruction requires 1 argument the label name", instruction.line_number);
        }

        impl::unconditional_jump(state, instruction.args[0], instruction.line_number);

        state.stack.ip++;
    }

    void jump_if_not_equal(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.size() < 3) {
            throw exceptions::VmError("Invalid Arguments: `jumpne` instruction requires 3 argument the variable name, what to compare the variable against and the label to jump to", instruction.line_number);
        }

        const auto [_, value] = state.get_variable<int32_t>(instruction.args[1], instruction.line_number);

        if (value != std::atoi(instruction.args[2].c_str()))// NOLINT(cert-err34-c)
        {
            impl::unconditional_jump(state, instruction.args[0], instruction.line_number);
        }
        else {
            state.stack.ip++;
        }
    }

    void ret(VmState &state, const Instruction &instruction)
    {
        if (state.call_stack_ptr <= 0) {
            throw exceptions::CallStackUnderflow(instruction.line_number, state.call_stack_ptr);
        }

        const auto call_site = state.call_stack[state.call_stack_ptr--];
        state.label_to_run = call_site.call_site_label;
        state.stack.ip = call_site.offset_from_start;

        state.stack.ip++;
    }

    void dec(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw exceptions::VmError("Invalid Arguments: `dec` instruction requires 1 argument the variable name", instruction.line_number);
        }

        impl::unary_op(state, instruction, [](int32_t value) { return --value; });

        state.stack.ip++;
    }

    void inc(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.empty()) {
            throw exceptions::VmError("Invalid Arguments: `inc` instruction requires 1 argument the variable name", instruction.line_number);
        }

        impl::unary_op(state, instruction, [](int32_t value) { return ++value; });

        state.stack.ip++;
    }

    void add(VmState &state, const Instruction &instruction)
    {

        if (instruction.args.size() < 2) {
            throw exceptions::VmError("Invalid Arguments: `add` instruction requires 2 arguments the operands of the addition", instruction.line_number);
        }

        if (instruction.args.size() == 3) {
            impl::binary_op<int32_t>(state, instruction, 2, std::plus<>());
        }
        else {
            impl::binary_op<int32_t>(state, instruction, 1, std::plus<>());
        }

        // Idk if this is necessary at all
        //        try {
        //        }
        //        catch ([[maybe_unused]] const exceptions::UndeclaredVariable &err) {
        //            const auto value = std::atoi(instruction.args[0].c_str()) + std::atoi(instruction.args[1].c_str());// NOLINT(cert-err34-c)
        //            state.set_variable(instruction.args[2], value, instruction.line_number);
        //        }

        state.stack.ip++;
    }

    void concat(VmState &state, const Instruction &instruction)
    {

        if (instruction.args.size() < 2) {
            throw exceptions::VmError("Invalid Arguments: `concat` instruction requires 2 arguments the operands of the concatenation", instruction.line_number);
        }

        if (instruction.args.size() == 3) {
            impl::binary_op<std::string>(state, instruction, 2, std::plus<>());
        }
        else {
            impl::binary_op<std::string>(state, instruction, 1, std::plus<>());
        }


        // Idk if this is necessary at all
        //        try {
        //        }
        //        catch ([[maybe_unused]] const exceptions::UndeclaredVariable &err) {
        //            state.set_variable(instruction.args[2], instruction.args[0] + instruction.args[1], instruction.line_number);
        //        }

        state.stack.ip++;
    }

    void mov(VmState &state, const Instruction &instruction)
    {
        if (instruction.args.size() < 2) {
            throw exceptions::VmError("Invalid Arguments: `mov` instruction requires 2 arguments the source variable and the destination variable", instruction.line_number);
        }

        const auto [it, value] = state.get_variable<int32_t>(instruction.args[0], instruction.line_number);
        state.set_variable(instruction.args[1], it->second, instruction.line_number);

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

        if (const auto integer = impl::get_v_opt<int32_t>(it->second); integer.has_value()) {
            std::cout << integer.value() << '\n';
        }
        else if (const auto string = impl::get_v_opt<std::string>(it->second); string.has_value()) {
            std::cout << string.value() << '\n';
        }
    }

    void unconditional_jump(VmState &state, const std::string &label_name, const std::size_t line_number)
    {
        const auto label = state.get_label("." + label_name, line_number);

        if (label.rbegin()[1] != "ret") {
            throw exceptions::NonReturningLabel(line_number, label_name);
        }

        if (state.call_stack_ptr == state.call_stack.max_size() - 1) {
            throw exceptions::CallStackOverflow(line_number, state.call_stack_ptr);
        }

        state.call_stack[++state.call_stack_ptr] = CallSite{.call_site_label = state.label_to_run, .offset_from_start = state.stack.ip};

        state.label_to_run = "." + label_name;
        state.stack.ip = 0;
    }

    template<typename Type, typename BinaryOp>
    void binary_op(VmState &state, const Instruction &instruction, const std::size_t output, BinaryOp binary_operation)
    {
        const auto [_it1, left] = state.get_variable<Type>(instruction.args[0], instruction.line_number);
        const auto [_it2, right] = state.get_variable<Type>(instruction.args[1], instruction.line_number);

        state.set_variable(instruction.args[output], binary_operation(left, right), instruction.line_number);
    }

    template<typename UnaryOp>
    void unary_op(VmState &state, const Instruction &instruction, UnaryOp unary_op)
    {
        auto [it, value] = state.get_variable<int32_t>(instruction.args[0], instruction.line_number);

        state.set_variable(it->first, unary_op(value), instruction.line_number);
    }

    template<typename T>
    std::optional<T> get_v_opt(const std::any &any)
    {
        if (const T *v = std::any_cast<T>(&any)) {
            return std::optional<T>(*v);
        }
        else {
            return std::nullopt;
        }
    }
}// namespace vm::instructions::impl
