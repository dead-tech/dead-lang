#include "Statement.hpp"

#include <fmt/format.h>
#include <utility>

namespace
{

[[nodiscard]] std::string expand_comma_separated_iterable(auto&& iterable, auto&& callable)
{
    return std::accumulate(
        iterable.begin(), iterable.end(), std::string{}, [&](const auto& acc, const auto& elem) {
            if (&elem != &iterable.back()) {
                return acc + callable(elem) + ", ";
            }

            return acc + callable(elem);
        });
}

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

[[nodiscard]] std::string compute_mutability(
    const Typechecker::VariableDeclaration& variable_declaration,
    const bool                              ignore_mutability = false)
{
    return !variable_declaration.is_mutable && !ignore_mutability ? "const " : "";
}

[[nodiscard]] std::string transpile_variable_declaration(
    const Typechecker::VariableDeclaration& variable_declaration,
    const bool                              ignore_mutability = false)
{
    const std::string mutability =
        compute_mutability(variable_declaration, ignore_mutability);
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
            if (const auto empty_statement = statement->template as<EmptyStatement>();
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
    const auto c_includes = std::accumulate(
        m_c_includes.begin(), m_c_includes.end(), std::string{}, [](const auto& acc, const auto& c_include) {
            return acc +
                   fmt::format("#include <{}>\n", c_include.substr(1, c_include.size() - 2));
        });

    const auto enums_code     = m_enums.evaluate();
    const auto structs_code   = m_structs.evaluate();
    const auto functions_code = m_functions.evaluate();

    return fmt::format("{}\n{}\n{}\n{}", c_includes, enums_code, structs_code, functions_code);
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
    // FIXME: Return value should be a proper type instead of a std::string
    const std::string return_value =
        Typechecker::builtin_type_from_string(m_return_type) != Typechecker::BuiltinType::NONE
            ? Typechecker::builtin_type_to_c_type(m_return_type)
            : m_return_type;

    const auto args = expand_comma_separated_iterable(m_args, [](const auto& arg) {
        return transpile_variable_declaration(arg);
    });

    return fmt::format(
        "{} {}({}) {{\n{}}}\n", return_value, m_name, args, m_body.evaluate());
}

IfStatement::IfStatement(std::shared_ptr<Expression> condition, BlockStatement then_block, BlockStatement else_block) noexcept
    : m_condition{std::move(condition)},
      m_then_block{std::move(then_block)},
      m_else_block{std::move(else_block)}
{
}

std::string IfStatement::evaluate() const noexcept
{
    const auto then_block = fmt::format(
        "if ({}) {{\n{}\n}}", m_condition->evaluate(), m_then_block.evaluate());

    const auto else_block = !m_else_block.empty()
                              ? fmt::format(" else {{\n{}\n}}", m_else_block.evaluate())
                              : "";

    return fmt::format("{}{}", then_block, else_block);
}

ReturnStatement::ReturnStatement(std::shared_ptr<Expression> expression) noexcept
    : m_expression{std::move(expression)}
{
}

std::string ReturnStatement::evaluate() const noexcept
{
    return fmt::format("return {};", m_expression->evaluate());
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
    return fmt::format("while ({}) {{\n{}\n}}", m_condition->evaluate(), m_body.evaluate());
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
    return fmt::format("{};", m_expression->evaluate());
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
    const auto array_elements = expand_comma_separated_iterable(
        m_elements, [](const auto& element) { return element->evaluate(); });

    return fmt::format(
        "{} = {{{}}};\n", transpile_variable_declaration(m_variable_declaration), array_elements);
}

StructStatement::StructStatement(std::string name, std::vector<Typechecker::VariableDeclaration> member_variables) noexcept
    : m_name{std::move(name)},
      m_member_variables{std::move(member_variables)}
{
}

std::string StructStatement::evaluate() const noexcept
{
    const auto member_variables = std::accumulate(
        m_member_variables.begin(),
        m_member_variables.end(),
        std::string{},
        [](const auto& acc, const auto& member_variable) {
            return acc + fmt::format("{};\n", transpile_variable_declaration(member_variable, true));
        });

    const auto default_constructor_params =
        expand_comma_separated_iterable(m_member_variables, [](const auto& member_variable) {
            return transpile_variable_declaration(member_variable, true);
        });

    const auto default_constructor_arguments =
        expand_comma_separated_iterable(m_member_variables, [](const auto& member_variable) {
            return fmt::format(".{} = {}", member_variable.name, member_variable.name);
        });

    const auto default_constructor_body =
        fmt::format("return {{ {} }};", default_constructor_arguments);

    const auto default_constructor = fmt::format(
        "static {} create({}) {{\n{}\n}}", m_name, default_constructor_params, default_constructor_body);

    return fmt::format("struct {} {{\n{}\n{}\n}};", m_name, member_variables, default_constructor);
}

EnumStatement::EnumStatement(std::string name, EnumStatement::EnumVariant variants) noexcept
    : m_name{std::move(name)},
      m_enum_variants{std::move(variants)}
{
}

std::string EnumStatement::evaluate() const noexcept
{
    std::string c_enum_code;

    const auto* const underlying_type = "unsigned long long int";

    const auto enum_variants = std::accumulate(
        m_enum_variants.begin(),
        m_enum_variants.end(),
        std::string{},
        [](const auto& acc, const auto& enum_variant) {
            return acc + fmt::format("{},\n", enum_variant.first);
        });

    const auto enum_code =
        fmt::format("enum class {} : {} {{\n{}\n}};", m_name, underlying_type, enum_variants);


    c_enum_code += fmt::format("struct __dl_{} {{\n", m_name);
    c_enum_code += fmt::format("    {} type;\n", m_name);
    c_enum_code += "    union {\n";

    std::stringstream associated_union_fields{};
    for (const auto& [name, fields] : m_enum_variants) {
        const auto struct_fields = std::accumulate(
            fields.begin(), fields.end(), std::string{}, [i = 0](const auto& acc, const auto& field) mutable {
                return acc + fmt::format("{} data_{};\n", transpile_type(field), i++);
            });
        associated_union_fields
            << fmt::format("struct {{ {} }} {}_data;\n", struct_fields, name);
    }

    const auto associated_union_code =
        fmt::format("union {{\n{}\n}};", associated_union_fields.str());

    std::stringstream associated_structs_default_constructors = {};
    for (const auto& [name, fields] : m_enum_variants) {
        const auto params = expand_comma_separated_iterable(
            fields, [&name, it = 0](const auto& field) mutable {
                return fmt::format("{} {}_{}", transpile_type(field), name, it++);
            });

        const auto arguments = expand_comma_separated_iterable(
            fields, [&name, it = 0]([[maybe_unused]] const auto& field) mutable {
                auto retval = fmt::format(".data_{} = {}_{}", it, name, it);
                it++;
                return retval;
            });

        const auto associated_struct_default_constructor_body = fmt::format(
            "return __dl_{} {{ .type = {}::{}, .{}_data = {{ {} }} }};", m_name, m_name, name, name, arguments);
        associated_structs_default_constructors << fmt::format(
            "static __dl_{} {}({}){{\n{}\n}}", m_name, name, params, associated_struct_default_constructor_body);
    }

    const auto associated_struct_code = fmt::format(
        "struct __dl_{} {{\n{} type;\n{}\n{}\n}};",
        m_name,
        m_name,
        associated_union_code,
        associated_structs_default_constructors.str());

    return fmt::format("{}\n{}\n", enum_code, associated_struct_code);
}

MatchStatement::MatchStatement(const std::shared_ptr<Expression>& expression, std::vector<MatchCase> cases) noexcept
    : m_expression{expression},
      m_cases{std::move(cases)}
{
}

std::string MatchStatement::evaluate() const noexcept
{
    std::stringstream match_cases = {};
    for (const auto& [label, destructuring, body] : m_cases) {
        std::string enum_variant;

        auto const* call_expression =
            label->enum_variant()->as<FunctionCallExpression>();
        if (call_expression != nullptr) {
            enum_variant = call_expression->function_name()->evaluate();
        } else {
            enum_variant = label->enum_variant()->evaluate();
        }

        const auto evaluated_label =
            fmt::format("{}::{}", label->enum_base()->evaluate(), enum_variant);

        if (evaluated_label != "_") {
            match_cases << fmt::format("case {}: {{\n", evaluated_label);
        } else {
            match_cases << fmt::format("default: {{\n");
        }

        const auto destructures = std::accumulate(
            destructuring.begin(),
            destructuring.end(),
            std::string{},
            [this, &enum_variant, i = 0](const auto& acc, const auto& destructure) mutable {
                return acc + fmt::format(
                                 "const auto {} = {}.{}_data.data_{};\n",
                                 destructure,
                                 m_expression->evaluate(),
                                 enum_variant,
                                 i++);
            });

        match_cases << fmt::format("{}\n{}break;\n}}\n", destructures, body.evaluate());
    }

    return fmt::format(
        "switch ({}.type) {{\n{}\n}}", m_expression->evaluate(), match_cases.str());
}
