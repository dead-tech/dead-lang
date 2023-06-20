#include "Statement.hpp"

#include <fmt/format.h>
#include <utility>

namespace
{


[[nodiscard]] std::string transpile_type(const Typechecker::Type& type)
{
    if (std::holds_alternative<Typechecker::BuiltinType>(type.variant())) {
        return Typechecker::builtin_type_to_c_type(
            std::get<Typechecker::BuiltinType>(type.variant()));
    }

    if (std::holds_alternative<Typechecker::CustomType>(type.variant())) {
        const auto custom_type = std::get<Typechecker::CustomType>(type.variant());

        if (custom_type.type == Token::Type::STRUCT) {
            return custom_type.name;
        }

        if (custom_type.type == Token::Type::ENUM) {
            return fmt::format("__dl_{}", custom_type.name);
        }
    }

    return "unreachable";
}

[[nodiscard]] std::string transpile_variable_declaration(
    const Typechecker::VariableDeclaration& variable_declaration,
    bool                                    ignore_mutability = false)
{
    const std::string mutability =
        variable_declaration.is_mutable || ignore_mutability ? "" : "const ";
    const std::string variable_type = transpile_type(variable_declaration.type);

    if (Typechecker::is_fixed_size_array(variable_declaration.type_extensions)) {
        return fmt::format(
            "{}{} {}{}",
            mutability,
            variable_type,
            variable_declaration.name,
            variable_declaration.type_extensions);
    }

    return fmt::format(
        "{}{}{} {}",
        mutability,
        variable_type,
        variable_declaration.type_extensions,
        variable_declaration.name);
}
} // namespace

std::string EmptyStatement::evaluate() const noexcept { return ""; }

BlockStatement::BlockStatement(std::vector<std::shared_ptr<Statement>> block) noexcept
    : m_block{std::move(block)}
{
}

std::string BlockStatement::evaluate() const noexcept
{
    return std::accumulate(
        m_block.begin(), m_block.end(), std::string{}, [](const auto& acc, const auto& statement) {
            // check if statement is EmptyStatement
            if (const auto empty_statement =
                    std::dynamic_pointer_cast<EmptyStatement>(statement);
                empty_statement) {
                return acc + statement->evaluate();
            }

            return acc + statement->evaluate() + "\n";
        });
}

auto BlockStatement::empty() const noexcept { return m_block.empty(); }

ModuleStatement::ModuleStatement(
    std::string              name,
    std::vector<std::string> c_includes,
    BlockStatement           structs,
    BlockStatement           enums,
    BlockStatement           functions) noexcept
    : m_name{std::move(name)},
      m_c_includes{std::move(c_includes)},
      m_structs{std::move(structs)},
      m_enums{std::move(enums)},
      m_functions{std::move(functions)}
{
}

std::string ModuleStatement::evaluate() const noexcept
{
    std::string c_module_code;

    for (const auto& c_include : m_c_includes) {
        c_module_code +=
            fmt::format("#include <{}>\n", c_include.substr(1, c_include.size() - 2));
    }
    c_module_code += "\n";

    c_module_code += m_enums.evaluate();
    c_module_code += "\n";

    c_module_code += m_structs.evaluate();
    c_module_code += "\n";

    c_module_code += m_functions.evaluate();

    return c_module_code;
}

FunctionStatement::FunctionStatement(
    std::string                                   name,
    std::vector<Typechecker::VariableDeclaration> args,
    std::string                                   return_type,
    BlockStatement                                body) noexcept
    : m_name{std::move(name)},
      m_args{std::move(args)},
      m_return_type{std::move(return_type)},
      m_body{std::move(body)}
{
}

std::string FunctionStatement::evaluate() const noexcept
{
    std::string c_function_code;

    const std::string return_value =
        Typechecker::builtin_type_from_string(m_return_type) != Typechecker::BuiltinType::NONE
            ? Typechecker::builtin_type_to_c_type(m_return_type)
            : m_return_type;

    std::string args;
    for (const auto& arg : m_args) {
        args += transpile_variable_declaration(arg);
        if (&arg != &m_args.back()) { args += ", "; }
    }

    c_function_code += fmt::format(
        "{} {}({}) {{\n{}}}\n", return_value, m_name, args, m_body.evaluate());
    return c_function_code;
}

IfStatement::IfStatement(std::shared_ptr<Expression> condition, BlockStatement then_block, BlockStatement else_block) noexcept
    : m_condition{std::move(condition)},
      m_then_block{std::move(then_block)},
      m_else_block{std::move(else_block)}
{
}

std::string IfStatement::evaluate() const noexcept
{
    std::string c_if_code;

    c_if_code += "if (" + m_condition->evaluate() + ") {\n";
    c_if_code += m_then_block.evaluate();

    if (!m_else_block.empty()) {
        c_if_code += "} else {\n";
        c_if_code += m_else_block.evaluate();
    }

    c_if_code += "}\n";

    return c_if_code;
}

ReturnStatement::ReturnStatement(std::shared_ptr<Expression> expression) noexcept
    : m_expression{std::move(expression)}
{
}

std::string ReturnStatement::evaluate() const noexcept
{
    return "return " + m_expression->evaluate() + ";";
}

VariableStatement::VariableStatement(
    Typechecker::VariableDeclaration variable,
    std::shared_ptr<Expression>      expression) noexcept
    : m_variable_declaration{std::move(variable)},
      m_expression{std::move(expression)}
{
}

std::string VariableStatement::evaluate() const noexcept
{
    return fmt::format(
        "{} = {};",
        transpile_variable_declaration(m_variable_declaration),
        m_expression->evaluate());
}

WhileStatement::WhileStatement(std::shared_ptr<Expression> condition, BlockStatement body) noexcept
    : m_condition{std::move(condition)},
      m_body{std::move(body)}
{
}

std::string WhileStatement::evaluate() const noexcept
{
    std::string c_while_code;

    c_while_code += "while (" + m_condition->evaluate() + ") {\n";
    c_while_code += m_body.evaluate();
    c_while_code += "}\n";

    return c_while_code;
}

ForStatement::ForStatement(
    std::shared_ptr<Statement>  init_statement,
    std::shared_ptr<Expression> condition,
    std::shared_ptr<Expression> increment_statement,
    BlockStatement              body) noexcept
    : m_init_statement{std::move(init_statement)},
      m_condition{std::move(condition)},
      m_increment_statement{std::move(increment_statement)},
      m_body{std::move(body)}
{
}

std::string ForStatement::evaluate() const noexcept
{
    return fmt::format(
        "for ({} {}; {}) {{\n{}}}\n",
        m_init_statement->evaluate(),
        m_condition->evaluate(),
        m_increment_statement->evaluate(),
        m_body.evaluate());
}

ExpressionStatement::ExpressionStatement(std::shared_ptr<Expression> expression) noexcept
    : m_expression{std::move(expression)}
{
}

std::string ExpressionStatement::evaluate() const noexcept
{
    return m_expression->evaluate() + ";";
}


ArrayStatement::ArrayStatement(
    Typechecker::VariableDeclaration         variable_declaration,
    std::vector<std::shared_ptr<Expression>> elements) noexcept
    : m_variable_declaration{std::move(variable_declaration)},
      m_elements{std::move(elements)}
{
}

std::string ArrayStatement::evaluate() const noexcept
{
    const std::string mutability = m_variable_declaration.is_mutable ? "" : "const ";

    std::string c_array_code;
    c_array_code +=
        fmt::format("{} = {{", transpile_variable_declaration(m_variable_declaration));

    for (const auto& element : m_elements) {
        c_array_code += element->evaluate();
        if (&element != &m_elements.back()) { c_array_code += ", "; }
    }

    c_array_code += "};";
    return c_array_code;
}

StructStatement::StructStatement(std::string name, std::vector<Typechecker::VariableDeclaration> member_variables) noexcept
    : m_name{std::move(name)},
      m_member_variables{std::move(member_variables)}
{
}

std::string StructStatement::evaluate() const noexcept
{
    std::string c_struct_code;

    c_struct_code += "typedef struct " + m_name + " {\n";
    for (const auto& member_variable : m_member_variables) {
        c_struct_code += fmt::format(
            "    {};\n", transpile_variable_declaration(member_variable, true));
    }
    c_struct_code += fmt::format("}} {};\n", m_name);

    // Automatically generate a constructor
    c_struct_code += fmt::format("\n{} {}_new(", m_name, m_name);
    for (const auto& member_variable : m_member_variables) {
        c_struct_code +=
            fmt::format("{}", transpile_variable_declaration(member_variable, true));
        if (&member_variable != &m_member_variables.back()) {
            c_struct_code += ", ";
        }
    }
    c_struct_code += ") {\n";

    std::string temporary_instance_name;
    std::ranges::transform(
        m_name, std::back_inserter(temporary_instance_name), [](const auto ch) {
            return std::tolower(ch);
        });
    c_struct_code += fmt::format("    {} {};\n", m_name, temporary_instance_name);

    for (const auto& member_variable : m_member_variables) {
        c_struct_code += fmt::format(
            "    {}.{} = {};\n",
            temporary_instance_name,
            member_variable.name,
            member_variable.name);
    }
    c_struct_code += fmt::format("    return {};\n", temporary_instance_name);
    c_struct_code += "}\n";

    return c_struct_code;
}

EnumStatement::EnumStatement(std::string name, EnumStatement::EnumVariant variants) noexcept
    : m_name{std::move(name)},
      m_variants{std::move(variants)}
{
}

std::string EnumStatement::evaluate() const noexcept
{
    std::string c_enum_code;

    c_enum_code += "typedef enum " + m_name + " {\n";
    for (const auto& [name, fields] : m_variants) {
        c_enum_code += fmt::format("    {}_{}_Var,\n", m_name, name);
    }
    c_enum_code += fmt::format("}} {};\n\n", m_name);

    c_enum_code += fmt::format("typedef struct __dl_{} {{\n", m_name);
    c_enum_code += fmt::format("    {} variant;\n", m_name);
    c_enum_code += "    union {\n";
    for (const auto& [name, fields] : m_variants) {
        const auto struct_fields = std::accumulate(
            fields.begin(), fields.end(), std::string{}, [i = 0](const auto& acc, const auto& field) mutable {
                return acc + transpile_type(field) + fmt::format(" data_{}", i++) + "; ";
            });

        c_enum_code += fmt::format(
            "        struct {{ {} }} {}_{}_Var_data;\n", struct_fields, m_name, name);
    }

    c_enum_code += "    };\n";
    c_enum_code += fmt::format("}} __dl_{};\n\n", m_name);

    for (const auto& [name, fields] : m_variants) {
        const auto params = std::accumulate(
            fields.begin(),
            fields.end(),
            std::string{},
            [&name, &fields, it = 0](const auto& acc, const auto& field) mutable {
                if (&field != &fields.back()) {
                    return acc + fmt::format("{} {}_{}, ", transpile_type(field), name, it++);
                }

                return acc + fmt::format("{} {}_{}", transpile_type(field), name, it++);
            });

        c_enum_code += fmt::format("__dl_{} {}_{}({}) {{\n", m_name, m_name, name, params);

        const auto arguments = std::accumulate(
            fields.begin(),
            fields.end(),
            std::string{},
            [&name, &fields, it = 0](const auto& acc, const auto& field) mutable {
                if (&field != &fields.back()) {
                    auto retval = acc + fmt::format(".data_{} = {}_{}, ", it, name, it);
                    it++;
                    return retval;
                }

                auto retval = acc + fmt::format(".data_{} = {}_{}", it, name, it);
                it++;
                return retval;
            });

        c_enum_code += fmt::format(
            "    const __dl_{} retval = {{ .variant = {}_{}_Var, "
            ".{}_{}_Var_data = {{ "
            "{} }}"
            "}};\n",
            m_name,
            m_name,
            name,
            m_name,
            name,
            arguments);
        c_enum_code += "\nreturn retval;\n}\n\n";
    }

    return c_enum_code;
}

MatchStatement::MatchStatement(
    std::shared_ptr<Statement>         enum_statement,
    const std::shared_ptr<Expression>& expression,
    std::vector<MatchCase>             cases) noexcept
    : m_enum{std::move(enum_statement)},
      m_expression{expression},
      m_cases{std::move(cases)}
{
}

std::string MatchStatement::evaluate() const noexcept
{
    std::string c_match_code;

    const auto enum_statement = std::dynamic_pointer_cast<EnumStatement>(m_enum);

    c_match_code += fmt::format("switch({}.variant) {{\n", m_expression->evaluate());

    for (const auto& [label, destructuring, body] : m_cases) {
        if (label != "_") {
            c_match_code +=
                fmt::format("case {}_{}_Var: {{\n", enum_statement->name(), label);
        } else {
            c_match_code += fmt::format("default: {{\n");
        }

        for (std::size_t i = 0; i < destructuring.size(); ++i) {
            const auto fields = enum_statement->variants().at(label);
            c_match_code += fmt::format(
                "const {} {} = {}.{}_{}_Var_data.data_{};",
                transpile_type(fields[i]),
                destructuring[i],
                m_expression->evaluate(),
                enum_statement->name(),
                label,
                i);
        }

        c_match_code += fmt::format("{}\n", body.evaluate());
        c_match_code += "break;\n}\n";
    }

    c_match_code += "}\n";

    return c_match_code;
}
