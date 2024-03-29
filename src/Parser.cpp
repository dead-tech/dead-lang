#include "Parser.hpp"

#include <dtsutil/filesystem.hpp>

#define ASSERT_OR_ERROR(condition, message, position) \
    if (!(condition)) {                               \
        m_supervisor->push_error(message, position);  \
        return nullptr;                               \
    }

#define MATCHES_OR_ERROR(token_type, message)                  \
    if (!matches_and_consume(token_type)) {                    \
        m_supervisor->push_error(message, peek()->position()); \
        return nullptr;                                        \
    }

std::vector<ModuleStatement>
Parser::parse(std::vector<Token> tokens, const std::shared_ptr<Supervisor>& supervisor) noexcept
{
    Parser parser(std::move(tokens), supervisor);
    return parser.parse_project();
}

Parser::Parser(std::vector<Token>&& tokens, const std::shared_ptr<Supervisor>& supervisor) noexcept
    : Iterator(tokens),
      m_supervisor{supervisor}
{
}

std::vector<ModuleStatement> Parser::parse_project() noexcept
{
    std::vector<ModuleStatement> modules;

    while (!eof() && !m_supervisor->has_errors()) {
        if (eol()) {
            advance(1);
            continue;
        }

        if (peek()->matches(Token::Type::IMPORT)) {
            advance(1); // Skip the import token

            const auto import_module = fmt::format("{}.dl", next()->lexeme());
            const auto import_module_path =
                m_supervisor->project_root().parent_path() / import_module;

            const auto module_content = dts::read_file(import_module_path.string());
            if (!module_content) {
                m_supervisor->push_error(
                    fmt::format("Could not import module: {}", import_module),
                    previous_position());
                return {};
            }

            const auto lexed_tokens = Lexer::lex(*module_content, m_supervisor);

            const auto imported_modules = Parser::parse(lexed_tokens, m_supervisor);
            modules.insert(
                modules.end(), imported_modules.begin(), imported_modules.end());
        }

        modules.push_back(*parse_module()->as<ModuleStatement>());
    }

    return modules;
}

std::shared_ptr<Statement> Parser::parse_module() noexcept
{
    std::string name = "main";

    std::vector<std::string>                c_includes;
    std::vector<std::shared_ptr<Statement>> structs;
    std::vector<std::shared_ptr<Statement>> enums;
    std::vector<std::shared_ptr<Statement>> functions;

    while (!eof() && !m_supervisor->has_errors()) {
        if (eol()) {
            advance(1);
            continue;
        }

        if (peek()->matches(Token::Type::MODULE)) {
            advance(1); // Skip the module token
            name = next()->lexeme();
        } else if (peek()->matches(Token::Type::C_INCLUDE)) {
            c_includes.push_back(parse_c_include_statement());
        } else if (peek()->matches(Token::Type::STRUCT)) {
            structs.push_back(parse_struct_statement());
        } else if (peek()->matches(Token::Type::ENUM)) {
            enums.push_back(parse_enum_statement());
        } else {
            functions.push_back(parse_function_statement());
        }
    }

    return std::make_shared<ModuleStatement>(
        name, c_includes, BlockStatement(structs), BlockStatement(enums), BlockStatement(functions));
}

std::shared_ptr<Statement> Parser::parse_function_statement() noexcept
{
    // Setup the new environment
    m_current_environment = std::make_unique<Environment>();

    // Skip the fn token
    const auto fn_token = next();

    const auto name = next();
    ASSERT_OR_ERROR(
        name && name->matches(Token::Type::IDENTIFIER),
        "expected function name after 'fn' keyword while parsing",
        fn_token->position())

    // Skip left paren
    MATCHES_OR_ERROR(Token::Type::LEFT_PAREN, "expected '(' after function name while parsing")

    // Parse arguments
    std::vector<Typechecker::VariableDeclaration> args;
    consume_tokens_until(Token::Type::RIGHT_PAREN, [this, &args] {
        if (peek()->matches(Token::Type::COMMA)) { advance(1); }
        args.push_back(parse_variable_declaration());
    });

    // Skip the right paren
    MATCHES_OR_ERROR(Token::Type::RIGHT_PAREN, "expected ')' after args while parsing")

    // Parse return type
    std::string return_type = "void";
    if (matches_and_consume(Token::Type::ARROW)) {
        return_type.clear();

        ASSERT_OR_ERROR(
            peek() && peek()->matches(Token::Type::IDENTIFIER),
            "expected return type after '->' while parsing",
            previous_position())

        consume_tokens_until(Token::Type::LEFT_BRACE, [this, &return_type] {
            return_type += next()->lexeme();
        });
    }

    // Skip the left brace
    MATCHES_OR_ERROR(Token::Type::LEFT_BRACE, "expected '{' after function return type while parsing")

    skip_newlines();

    // Parse body
    const auto body = parse_statement_block();
    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACE, "expected '}' after function body while parsing")

    return std::make_shared<FunctionStatement>(
        FunctionStatement(name->lexeme(), args, return_type, BlockStatement(body)));
}

std::shared_ptr<Statement> Parser::parse_statement()
{
    switch (peek()->type()) {
        case Token::Type::IF: {
            return parse_if_statement();
        }
        case Token::Type::RETURN: {
            return parse_return_statement();
        }
        case Token::Type::MUT:
        case Token::Type::IDENTIFIER: {
            if (identifier_is_function_call()) {
                return parse_expression_statement();
            }
            return parse_variable_statement();
        }
        case Token::Type::WHILE: {
            return parse_while_statement();
        }
        case Token::Type::FOR: {
            return parse_for_statement();
        }
        case Token::Type::MATCH: {
            return parse_match_statement();
        }
        case Token::Type::END_OF_LINE: {
            advance(1);
            return std::make_shared<EmptyStatement>();
        }
        default: {
            return parse_expression_statement();
        }
    }
}

std::shared_ptr<Statement> Parser::parse_if_statement()
{
    // Skip the if token
    const auto if_token = next();

    // Skip the left paren
    MATCHES_OR_ERROR(Token::Type::LEFT_PAREN, "expected '(' after if keyword while parsing")

    // Parse condition
    const auto condition = parse_expression();
    MATCHES_OR_ERROR(Token::Type::RIGHT_PAREN, "expected ')' after if condition while parsing")
    ASSERT_OR_ERROR(
        condition,
        "expected expression while parsing if statement condition",
        if_token->position())

    // Skip the left brace
    MATCHES_OR_ERROR(Token::Type::LEFT_BRACE, "expected '{' after if condition while parsing")

    // Parse then block
    const auto then_block = parse_statement_block();

    // Skip then block right brace
    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACE, "expected '}' after if statement's 'then branch' while parsing")

    // Parse else block
    if (const auto else_token = peek(); !else_token || else_token->lexeme() != "else") {
        return std::make_shared<IfStatement>(
            IfStatement(condition, BlockStatement(then_block), BlockStatement({})));
    }

    advance(1); // Skip the else token
    MATCHES_OR_ERROR(Token::Type::LEFT_BRACE, "expected '{' after if statement's 'else branch' while parsing")

    const auto else_block = parse_statement_block();
    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACE, "expected '}' after if statement's 'else branch' while parsing")

    return std::make_shared<IfStatement>(IfStatement(
        condition, BlockStatement(then_block), BlockStatement(else_block)));
}

std::shared_ptr<Statement> Parser::parse_return_statement()
{
    // Skip the return token
    const auto return_token = next();

    // Parse expression
    const auto expression = parse_expression();
    ASSERT_OR_ERROR(
        expression,
        "expected expression after return keyword while parsing",
        return_token->position())

    return std::make_shared<ReturnStatement>(ReturnStatement(expression));
}

std::shared_ptr<Statement> Parser::parse_variable_statement(const Token::Type& ending_delimiter)
{
    if (!Typechecker::is_valid_type(peek()->lexeme(), m_custom_types) &&
        !peek()->matches(Token::Type::MUT)) {
        return std::make_shared<ExpressionStatement>(parse_assignment_expression());
    }

    const auto variable_declaration = parse_variable_declaration();
    if (Typechecker::is_fixed_size_array(variable_declaration.type_extensions)) {
        return parse_array_statement(variable_declaration);
    }

    // Skip equal sign
    const auto equal_token = peek();
    MATCHES_OR_ERROR(Token::Type::EQUAL, "expected '=' after variable name while parsing")

    const auto expression = parse_expression();
    ASSERT_OR_ERROR(
        expression,
        "expected expression after '=' in variable declaration while parsing",
        equal_token->position())

    MATCHES_OR_ERROR(ending_delimiter, "expected ';' or newline after expression in variable declaration while parsing")

    m_current_environment->enscope(variable_declaration);

    return std::make_shared<VariableStatement>(
        VariableStatement(variable_declaration, expression));
}

std::shared_ptr<Statement> Parser::parse_while_statement()
{
    // Skip the while token and the left paren
    const auto while_token = next();

    MATCHES_OR_ERROR(Token::Type::LEFT_PAREN, "expected '(' after while keyword while parsing")

    // Parse condition
    const auto condition = parse_expression();

    // Skip the right paren
    MATCHES_OR_ERROR(Token::Type::RIGHT_PAREN, "expected ')' after while-loop condition while parsing")

    ASSERT_OR_ERROR(
        condition,
        "expected expression while parsing while-loop condition",
        while_token->position())

    // Skip the left brace
    MATCHES_OR_ERROR(Token::Type::LEFT_BRACE, "expected '{' after while-loop condition while parsing")

    // Parse body
    const auto body = parse_statement_block();

    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACE, "expected '}' after while-loop body while parsing")

    return std::make_shared<WhileStatement>(WhileStatement(condition, BlockStatement(body)));
}

std::shared_ptr<Statement> Parser::parse_for_statement()
{
    // Skip the for token and the left paren
    const auto for_token = next();

    MATCHES_OR_ERROR(Token::Type::LEFT_PAREN, "expected '(' after for keyword while parsing")

    // Parse initializer
    const auto initializer = parse_variable_statement(Token::Type::SEMICOLON);
    ASSERT_OR_ERROR(
        initializer,
        "expected variable declaration while parsing for-loop initializer",
        for_token->position())

    // Parse condition
    const auto condition = parse_expression();
    ASSERT_OR_ERROR(
        condition,
        "expected expression while parsing for-loop condition",
        for_token->position())

    MATCHES_OR_ERROR(Token::Type::SEMICOLON, "expected ';' after for-loop condition while parsing")

    // Parse increment
    const auto increment = parse_expression();

    // Skip the right paren
    MATCHES_OR_ERROR(Token::Type::RIGHT_PAREN, "expected ')' after for-loop increment while parsing")

    ASSERT_OR_ERROR(
        increment,
        "expected expression while parsing for-loop increment",
        for_token->position())

    // Skip the left brace
    MATCHES_OR_ERROR(Token::Type::LEFT_BRACE, "expected '{' after for-loop increment while parsing")

    // Parse body
    const auto body = parse_statement_block();

    // Skip the right brace
    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACE, "expected '}' after for-loop body while parsing")

    return std::make_shared<ForStatement>(
        ForStatement(initializer, condition, increment, BlockStatement(body)));
}

std::shared_ptr<Statement> Parser::parse_expression_statement()
{
    const auto expression = parse_expression();
    ASSERT_OR_ERROR(expression, "expected expression while parsing expression statement", previous_position())
    skip_newlines();
    return std::make_shared<ExpressionStatement>(ExpressionStatement(expression));
}

std::shared_ptr<Statement> Parser::parse_array_statement(const Typechecker::VariableDeclaration& variable_declaration)
{
    MATCHES_OR_ERROR(Token::Type::EQUAL, "expected '=' after array declaration while parsing")

    MATCHES_OR_ERROR(Token::Type::LEFT_BRACKET, "expected '[' after array declaration while parsing")

    std::vector<std::shared_ptr<Expression>> array_elements;
    consume_tokens_until(Token::Type::RIGHT_BRACKET, [this, &array_elements] {
        if (peek()->matches(Token::Type::COMMA)) { advance(1); }
        const auto expression = parse_expression();
        if (!expression) { return; }
        array_elements.push_back(expression);
    });

    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACKET, "expected ']' after array declaration while parsing")

    skip_newlines();

    return std::make_shared<ArrayStatement>(ArrayStatement(variable_declaration, array_elements));
}

std::string Parser::parse_c_include_statement()
{
    const auto include_token = next();
    const auto path          = next();

    if (!path || !path->matches(Token::Type::DOUBLE_QUOTED_STRING)) {
        m_supervisor->push_error(
            "expected path after 'include' while parsing", include_token->position());
        return "";
    }

    return path->lexeme();
}

std::shared_ptr<Statement> Parser::parse_struct_statement() noexcept
{
    const auto struct_token = next();

    const auto struct_name = parse_identifier();
    if (struct_name.empty()) { return nullptr; }

    MATCHES_OR_ERROR(Token::Type::LEFT_BRACE, "expected '{' after struct name while parsing")

    skip_newlines();

    const std::vector<Typechecker::VariableDeclaration> member_variables =
        parse_member_variables();

    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACE, "expected '}' after struct body while parsing")

    skip_newlines();

    const auto struct_statement =
        std::make_shared<StructStatement>(StructStatement(struct_name, member_variables));

    m_custom_types.emplace(Typechecker::CustomType(struct_name, Token::Type::STRUCT), struct_statement);

    return struct_statement;
}

std::shared_ptr<Statement> Parser::parse_enum_statement() noexcept
{
    const auto enum_token = next();

    const auto enum_name = parse_identifier();
    ASSERT_OR_ERROR(
        !enum_name.empty(),
        "expected identifier after enum keyword while parsing",
        enum_token->position())

    MATCHES_OR_ERROR(Token::Type::LEFT_BRACE, "expected '{' after enum name while parsing")

    skip_newlines();

    const auto enum_variants = parse_enum_variants();

    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACE, "expected '}' after enum variants while parsing")

    skip_newlines();

    const auto enum_statement =
        std::make_shared<EnumStatement>(EnumStatement(enum_name, enum_variants));

    m_custom_types.emplace(Typechecker::CustomType(enum_name, Token::Type::ENUM), enum_statement);

    return enum_statement;
}

std::shared_ptr<Statement> Parser::parse_match_statement() noexcept
{
    const auto match_token = next();

    MATCHES_OR_ERROR(Token::Type::LEFT_PAREN, "expected '(' after match keyword while parsing")

    const auto match_expression = parse_expression();
    ASSERT_OR_ERROR(
        match_expression,
        "expected expression after match keyword while parsing",
        match_token->position())

    MATCHES_OR_ERROR(Token::Type::RIGHT_PAREN, "expected ')' after match expression while parsing")

    MATCHES_OR_ERROR(Token::Type::LEFT_BRACE, "expected '{' after match expression while parsing")

    skip_newlines();

    // FIXME: Error handling has to be improved a lot here.
    std::vector<MatchStatement::MatchCase> match_cases;
    std::vector<std::string>               destructuring;

    consume_tokens_until(Token::Type::RIGHT_BRACE, [this, &match_cases, &destructuring] {
        const auto label = parse_expression();

        auto* const enum_expression = label->as<EnumExpression>();
        if (enum_expression == nullptr) {
            m_supervisor->push_error(
                "expected enum variant while parsing match cases", previous_position());
            return;
        }

        auto* const call_expression =
            enum_expression->enum_variant()->as<FunctionCallExpression>();

        // Destructuring
        if (call_expression != nullptr) {
            for (const auto& argument : call_expression->arguments()) {
                destructuring.push_back(argument->evaluate());
            }
        }

        if (!matches_and_consume(Token::Type::FAT_ARROW)) {
            m_supervisor->push_error(
                "expected '->' after match label while parsing", previous_position());
            return;
        }

        if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
            m_supervisor->push_error(
                "expected '{' after match label while parsing", previous_position());
            return;
        }

        const auto body = parse_statement_block();

        if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
            m_supervisor->push_error(
                "expected '}' after match body while parsing", previous_position());
            return;
        }

        skip_newlines();

        match_cases.emplace_back(
            std::make_shared<EnumExpression>(*enum_expression),
            destructuring,
            BlockStatement(body));
        destructuring.clear();
    });

    ASSERT_OR_ERROR(
        !match_cases.empty(),
        "expected at least one match case while parsing",
        match_token->position())

    MATCHES_OR_ERROR(Token::Type::RIGHT_BRACE, "expected '}' after match cases while parsing")

    skip_newlines();

    return std::make_shared<MatchStatement>(MatchStatement(match_expression, match_cases));
}

std::shared_ptr<Expression> Parser::parse_expression()
{
    return parse_assignment_expression();
}

std::shared_ptr<Expression> Parser::parse_assignment_expression()
{
    auto expression = parse_logical_expression();

    if (const auto assignment_operator = peek();
        Token::is_assignment_operator(*assignment_operator)) {
        advance(1); // Skip the assignment operator

        auto value = parse_assignment_expression();
        ASSERT_OR_ERROR(
            value,
            "expected expression after assignment operator while parsing",
            assignment_operator->position())

        if (Typechecker::is_valid_lvalue(expression)) {
            return std::make_shared<AssignmentExpression>(AssignmentExpression(
                expression, assignment_operator->type(), std::move(value)));
        }


        m_supervisor->push_error(
            "expected variable on left side of assignment while parsing",
            previous_position());
    }

    return expression;
}

std::shared_ptr<Expression> Parser::parse_logical_expression()
{
    auto expression = parse_equality_expression();

    auto logical_operator = peek();
    while (Token::is_logical_operator(*logical_operator)) {
        advance(1); // Skip the logical operator

        auto right = parse_equality_expression();
        ASSERT_OR_ERROR(
            right,
            fmt::format(
                "expected expression after logical operator while parsing",
                logical_operator->lexeme()),
            logical_operator->position())

        expression = std::make_shared<LogicalExpression>(LogicalExpression(
            std::move(expression), logical_operator->type(), std::move(right)));
        logical_operator = peek();
    }

    return expression;
}

std::shared_ptr<Expression> Parser::parse_equality_expression()
{
    auto expression = parse_comparison_expression();

    auto equality_operator = peek();
    while (Token::is_equality_operator(*equality_operator)) {
        advance(1);
        auto right = parse_comparison_expression();
        ASSERT_OR_ERROR(
            right,
            "expected expression after equality operator while parsing",
            equality_operator->position())

        expression        = std::make_shared<BinaryExpression>(BinaryExpression(
            std::move(expression), equality_operator->type(), std::move(right)));
        equality_operator = peek();
    }

    return expression;
}

std::shared_ptr<Expression> Parser::parse_comparison_expression()
{
    auto expression = parse_arithmetic_operator_expression();

    auto comparison_operator = peek();
    while (Token::is_comparison_operator(*comparison_operator)) {
        advance(1);
        auto right = parse_arithmetic_operator_expression();
        ASSERT_OR_ERROR(
            right,
            "expected expression after comparison operator while parsing",
            comparison_operator->position())

        expression = std::make_shared<BinaryExpression>(BinaryExpression(
            std::move(expression), comparison_operator->type(), std::move(right)));
        comparison_operator = peek();
    }

    return expression;
}

std::shared_ptr<Expression> Parser::parse_arithmetic_operator_expression()
{
    auto expression = parse_index_operator_expression();

    auto arithmetic_operator = peek();
    while (Token::is_arithmetic_operator(*arithmetic_operator)) {
        advance(1); // Skip the arithmetic operator

        auto right = parse_index_operator_expression();
        ASSERT_OR_ERROR(
            right,
            fmt::format(
                "expected expression after '{}' arithmetic operator while "
                "parsing",
                arithmetic_operator->lexeme()),
            arithmetic_operator->position())

        expression = std::make_shared<BinaryExpression>(BinaryExpression(
            std::move(expression), arithmetic_operator->type(), std::move(right)));
        arithmetic_operator = peek();
    }

    return expression;
}

std::shared_ptr<Expression> Parser::parse_index_operator_expression()
{
    auto expression = parse_field_accessors_expression();

    while (matches_and_consume(Token::Type::LEFT_BRACKET)) {
        auto index = parse_expression();
        ASSERT_OR_ERROR(index, "expected expression inside index operator while parsing", previous_position())

        MATCHES_OR_ERROR(Token::Type::RIGHT_BRACKET, "expected ']' after index operator while parsing")

        expression = std::make_shared<IndexOperatorExpression>(
            IndexOperatorExpression(std::move(expression), std::move(index)));
    }

    return expression;
}

std::shared_ptr<Expression> Parser::parse_field_accessors_expression()
{
    auto expression = parse_unary_expression();

    auto field_accessor = peek();
    while (Token::is_field_accessor(*field_accessor)) {
        advance(1);
        auto right = parse_unary_expression();
        ASSERT_OR_ERROR(
            right,
            fmt::format(
                "expected expression after '{}' while parsing", field_accessor->lexeme()),
            field_accessor->position())

        // Check if it is an enum accessor
        const auto custom_type_name = expression->evaluate();
        const auto custom_type_key =
            Typechecker::CustomType(custom_type_name, Token::Type::ENUM);

        if (m_custom_types.contains(custom_type_key)) {
            expression = std::make_shared<EnumExpression>(
                EnumExpression(std::move(expression), std::move(right)));
            field_accessor = peek();
            continue;
        }

        expression     = std::make_shared<BinaryExpression>(BinaryExpression(
            std::move(expression), field_accessor->type(), std::move(right)));
        field_accessor = peek();
    }

    return expression;
}

std::shared_ptr<Expression> Parser::parse_unary_expression()
{
    if (const auto unary_operator = peek(); Token::is_unary_operator(*unary_operator)) {
        advance(1); // consume the operator
        auto right = parse_unary_expression();
        ASSERT_OR_ERROR(
            right,
            "expected expression after unary operator while parsing",
            unary_operator->position())

        return std::make_shared<UnaryExpression>(
            UnaryExpression(unary_operator->type(), std::move(right)));
    }

    return parse_function_call_expression();
}

std::shared_ptr<Expression> Parser::parse_function_call_expression()
{
    auto identifier = parse_primary_expression();

    if (!matches_and_consume(Token::Type::LEFT_PAREN)) { return identifier; }

    std::vector<std::shared_ptr<Expression>> arguments;
    consume_tokens_until(Token::Type::RIGHT_PAREN, [this, &arguments] {
        if (peek()->matches(Token::Type::COMMA)) { advance(1); }
        arguments.push_back(parse_expression());
    });

    MATCHES_OR_ERROR(Token::Type::RIGHT_PAREN, "expected ')' after function call while parsing")

    return std::make_shared<FunctionCallExpression>(
        FunctionCallExpression(identifier, arguments));
}

std::shared_ptr<Expression> Parser::parse_primary_expression()
{
    const auto current_token = next();

    if (Token::is_literal(*current_token)) {
        return std::make_shared<LiteralExpression>(current_token->lexeme());
    }

    if (Token::is_boolean(*current_token)) {
        return std::make_shared<LiteralExpression>(current_token->lexeme());
    }

    if (current_token->matches(Token::Type::IDENTIFIER)) {
        return std::make_shared<VariableExpression>(current_token->lexeme());
    }

    if (current_token->matches(Token::Type::LEFT_PAREN)) {
        auto expression = parse_expression();
        MATCHES_OR_ERROR(Token::Type::RIGHT_PAREN, "expected ')' after expression while parsing")
        return std::make_shared<GroupingExpression>(std::move(expression));
    }

    m_supervisor->push_error(
        fmt::format("unexpected token '{}' while parsing", current_token->lexeme()),
        current_token->position());
    return nullptr;
}

std::vector<std::shared_ptr<Statement>> Parser::parse_statement_block() noexcept
{
    m_current_environment = std::make_shared<Environment>(m_current_environment);

    std::vector<std::shared_ptr<Statement>> block;
    consume_tokens_until(Token::Type::RIGHT_BRACE, [this, &block] {
        block.push_back(parse_statement());
    });

    m_current_environment = m_current_environment->parent();

    return block;
}

std::string Parser::parse_identifier() noexcept
{
    const auto identifier = next();
    if (!identifier || !identifier->matches(Token::Type::IDENTIFIER)) {
        const auto previous_token = peek_behind(2);
        m_supervisor->push_error(
            fmt::format(
                "expected identifier after '{}' while parsing", previous_token->lexeme()),
            previous_token->position());
        return "";
    }

    return identifier->lexeme();
}

std::vector<Typechecker::VariableDeclaration> Parser::parse_member_variables() noexcept
{
    std::vector<Typechecker::VariableDeclaration> member_variables;
    consume_tokens_until(Token::Type::RIGHT_BRACE, [this, &member_variables] {
        member_variables.push_back(parse_variable_declaration());
    });

    return member_variables;
}

Typechecker::VariableDeclaration Parser::parse_variable_declaration() noexcept
{
    const bool is_mutable = peek()->type() == Token::Type::MUT;

    // Skip the mut keyword if present
    if (is_mutable) { advance(1); }

    auto variable_type = Typechecker::builtin_type_from_string(peek()->lexeme());
    std::optional<Typechecker::CustomType> custom_type =
        defined_custom_type(peek()->lexeme());

    if (variable_type == Typechecker::BuiltinType::NONE && !custom_type) {
        m_supervisor->push_error(
            fmt::format("expected variable while parsing", peek()->lexeme()),
            peek()->position());
    }

    // Skip the type
    advance(1);

    std::string type_extensions;
    consume_tokens_until(Token::Type::IDENTIFIER, [this, &type_extensions] {
        if (eol()) {
            m_supervisor->push_error(
                "expected variable name after variable type while parsing",
                previous_position());
        }
        type_extensions.append(next()->lexeme());
    });

    const std::string variable_name = parse_identifier();

    skip_newlines();

    return Typechecker::VariableDeclaration{
        .is_mutable = is_mutable,
        .type = custom_type ? Typechecker::Type(*custom_type) : Typechecker::Type(variable_type),
        .type_extensions = type_extensions,
        .name            = variable_name,
    };
}

EnumStatement::EnumVariant Parser::parse_enum_variants() noexcept
{
    EnumStatement::EnumVariant variants;
    consume_tokens_until(Token::Type::RIGHT_BRACE, [this, &variants] {
        const auto variant_name = parse_identifier();
        if (variant_name.empty()) {
            m_supervisor->push_error(
                "expected enum variant name while parsing", previous_position());
            return;
        }

        // This enum variants has fields
        std::vector<Typechecker::Type> fields;
        if (peek()->matches(Token::Type::LEFT_PAREN)) {
            // Skip the left paren
            advance(1);

            consume_tokens_until(Token::Type::RIGHT_PAREN, [this, &fields] {
                if (peek()->matches(Token::Type::COMMA)) { advance(1); }

                const auto field_type = parse_identifier();
                if (field_type.empty()) {
                    m_supervisor->push_error(
                        "expected field name while parsing", previous_position());
                    return;
                }

                if (!Typechecker::is_valid_type(field_type, m_custom_types)) {
                    m_supervisor->push_error(
                        fmt::format("{} is not a valid type while parsing enum variant", field_type),
                        previous_position());
                    return;
                }

                fields.push_back(Typechecker::resolve_type(field_type, Token::Type::ENUM));
            });

            if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
                m_supervisor->push_error(
                    "expected ')' after enum variant fields while parsing",
                    previous_position());
                return;
            }
        }

        skip_newlines();

        variants.emplace(variant_name, fields);
    });

    return variants;
}

Position Parser::previous_position() const noexcept
{
    return previous().value_or(Token::create_dumb()).position();
}

template <std::invocable Callable>
void Parser::consume_tokens_until(const Token::Type& delimiter, Callable&& callable) noexcept
{
    while (!peek()->matches(delimiter)) {
        if (eof() || m_supervisor->has_errors()) { return; }
        callable();
    }
}

bool Parser::matches_and_consume(const Token::Type& delimiter) noexcept
{
    if (const auto token = peek(); !peek() || !peek()->matches(delimiter)) {
        return false;
    }

    advance(1);
    return true;
}

bool Parser::eol() const noexcept
{
    return peek()->matches(Token::Type::END_OF_LINE);
}

void Parser::skip_newlines() noexcept
{
    while (eol()) { advance(1); }
}

bool Parser::identifier_is_function_call() const noexcept
{
    if (const auto token = peek_ahead(1); !token || !token->matches(Token::Type::LEFT_PAREN)) {
        return false;
    }
    return true;
}

std::optional<Typechecker::CustomType> Parser::defined_custom_type(const std::string token) const noexcept
{
    const auto found = std::ranges::find_if(m_custom_types, [&token](const auto map_entry) {
        const auto [custom_type, statement] = map_entry;
        return custom_type.name == token;
    });

    if (found != m_custom_types.end()) { return found->first; }
    return {};
}
