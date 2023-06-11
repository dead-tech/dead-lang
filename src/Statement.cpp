#include "Statement.hpp"

#include <fmt/format.h>
#include <utility>

namespace {
std::string transpile_variable_declaration(
  const Typechecker::VariableDeclaration& variable_declaration,
  bool                                    ignore_mutability = false
) {
    const std::string mutability    = variable_declaration.is_mutable || ignore_mutability ? "" : "const ";
    const std::string variable_type = variable_declaration.type == Typechecker::BuiltinType::STRUCT
                                        ? *variable_declaration.custom_type
                                        : Typechecker::builtin_type_to_c_type(variable_declaration.type);

    if (Typechecker::is_fixed_size_array(variable_declaration.type_extensions)) {
        return fmt::format(
          "{}{} {}{}", mutability, variable_type, variable_declaration.name, variable_declaration.type_extensions
        );
    }

    return fmt::format(
      "{}{}{} {}", mutability, variable_type, variable_declaration.type_extensions, variable_declaration.name
    );
}
} // namespace

std::string EmptyStatement::evaluate() const noexcept { return ""; }

BlockStatement::BlockStatement(std::vector<std::shared_ptr<Statement>> block) noexcept
  : m_block{ std::move(block) } {}

std::string BlockStatement::evaluate() const noexcept {
    return std::accumulate(m_block.begin(), m_block.end(), std::string{}, [](const auto& acc, const auto& statement) {
        // check if statement is EmptyStatement
        if (const auto empty_statement = std::dynamic_pointer_cast<EmptyStatement>(statement); empty_statement) {
            return acc + statement->evaluate();
        } else {
            return acc + statement->evaluate() + "\n";
        }
    });
}

auto BlockStatement::empty() const noexcept { return m_block.empty(); }

void BlockStatement::append(const std::shared_ptr<Statement>& statement) noexcept { m_block.push_back(statement); }

ModuleStatement::ModuleStatement(
  std::string              name,
  std::vector<std::string> c_includes,
  BlockStatement           structs,
  BlockStatement           functions
) noexcept
  : m_name{ std::move(name) },
    m_c_includes{ std::move(c_includes) },
    m_structs{ std::move(structs) },
    m_functions{ std::move(functions) } {}

std::string ModuleStatement::evaluate() const noexcept {
    std::string c_module_code;

    for (const auto& c_include : m_c_includes) {
        c_module_code += fmt::format("#include <{}>\n", c_include.substr(1, c_include.size() - 2));
    }
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
  BlockStatement                                body
) noexcept
  : m_name{ std::move(name) },
    m_args{ std::move(args) },
    m_return_type{ std::move(return_type) },
    m_body{ std::move(body) } {}

std::string FunctionStatement::evaluate() const noexcept {
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

    c_function_code += fmt::format("{} {}({}) {{\n{}}}\n", return_value, m_name, args, m_body.evaluate());
    return c_function_code;
}

IfStatement::IfStatement(
  std::shared_ptr<Expression> condition,
  BlockStatement              then_block,
  BlockStatement              else_block
) noexcept
  : m_condition{ std::move(condition) },
    m_then_block{ std::move(then_block) },
    m_else_block{ std::move(else_block) } {}

std::string IfStatement::evaluate() const noexcept {
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
  : m_expression{ std::move(expression) } {}

std::string ReturnStatement::evaluate() const noexcept { return "return " + m_expression->evaluate() + ";"; }

VariableStatement::VariableStatement(
  Typechecker::VariableDeclaration variable,
  std::shared_ptr<Expression>      expression
) noexcept
  : m_variable_declaration{ std::move(variable) },
    m_expression{ std::move(expression) } {}

std::string VariableStatement::evaluate() const noexcept {
    return fmt::format("{} = {};", transpile_variable_declaration(m_variable_declaration), m_expression->evaluate());
}

PlusEqualStatement::PlusEqualStatement(std::string name, std::shared_ptr<Expression> expression) noexcept
  : m_name{ std::move(name) },
    m_expression{ std::move(expression) } {}

std::string PlusEqualStatement::evaluate() const noexcept { return m_name + " += " + m_expression->evaluate() + ";"; }

WhileStatement::WhileStatement(std::shared_ptr<Expression> condition, BlockStatement body) noexcept
  : m_condition{ std::move(condition) },
    m_body{ std::move(body) } {}

std::string WhileStatement::evaluate() const noexcept {
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
  BlockStatement              body
) noexcept
  : m_init_statement{ std::move(init_statement) },
    m_condition{ std::move(condition) },
    m_increment_statement{ std::move(increment_statement) },
    m_body{ std::move(body) } {}

std::string ForStatement::evaluate() const noexcept {
    return fmt::format(
      "for ({} {}; {}) {{\n{}}}\n",
      m_init_statement->evaluate(),
      m_condition->evaluate(),
      m_increment_statement->evaluate(),
      m_body.evaluate()
    );
}

ExpressionStatement::ExpressionStatement(std::shared_ptr<Expression> expression) noexcept
  : m_expression{ std::move(expression) } {}

std::string ExpressionStatement::evaluate() const noexcept { return m_expression->evaluate() + ";"; }


ArrayStatement::ArrayStatement(
  Typechecker::VariableDeclaration         variable_declaration,
  std::vector<std::shared_ptr<Expression>> elements
) noexcept
  : m_variable_declaration{ std::move(variable_declaration) },
    m_elements{ std::move(elements) } {}

std::string ArrayStatement::evaluate() const noexcept {
    const std::string mutability = m_variable_declaration.is_mutable ? "" : "const ";

    std::string c_array_code;
    c_array_code += fmt::format("{} = {{", transpile_variable_declaration(m_variable_declaration));

    for (const auto& element : m_elements) {
        c_array_code += element->evaluate();
        if (&element != &m_elements.back()) { c_array_code += ", "; }
    }

    c_array_code += "};";
    return c_array_code;
}

IndexOperatorStatement::IndexOperatorStatement(
  std::string                 variable_name,
  std::shared_ptr<Expression> index,
  std::shared_ptr<Expression> expression
) noexcept
  : m_variable_name{ std::move(variable_name) },
    m_index{ std::move(index) },
    m_expression{ std::move(expression) } {}

std::string IndexOperatorStatement::evaluate() const noexcept {
    return fmt::format("{}[{}] = {};", m_variable_name, m_index->evaluate(), m_expression->evaluate());
}

StructStatement::StructStatement(
  std::string                                   name,
  std::vector<Typechecker::VariableDeclaration> member_variables
) noexcept
  : m_name{ std::move(name) },
    m_member_variables{ std::move(member_variables) } {}

std::string StructStatement::evaluate() const noexcept {
    std::string c_struct_code;

    c_struct_code += "typedef struct " + m_name + " {\n";
    for (const auto& member_variable : m_member_variables) {
        c_struct_code += fmt::format("    {};\n", transpile_variable_declaration(member_variable, true));
    }
    c_struct_code += fmt::format("}} {};\n", m_name);

    // Automatically generate a constructor
    c_struct_code += fmt::format("\n{} {}_new(", m_name, m_name);
    for (const auto& member_variable : m_member_variables) {
        c_struct_code += fmt::format("{}", transpile_variable_declaration(member_variable));
        if (&member_variable != &m_member_variables.back()) { c_struct_code += ", "; }
    }
    c_struct_code += ") {\n";

    std::string temporary_instance_name;
    std::ranges::transform(m_name, std::back_inserter(temporary_instance_name), [](const auto ch) {
        return std::tolower(ch);
    });
    c_struct_code += fmt::format("    {} {};\n", m_name, temporary_instance_name);

    for (const auto& member_variable : m_member_variables) {
        c_struct_code +=
          fmt::format("    {}.{} = {};\n", temporary_instance_name, member_variable.name, member_variable.name);
    }
    c_struct_code += fmt::format("    return {};\n", temporary_instance_name);
    c_struct_code += "}\n";

    return c_struct_code;
}
