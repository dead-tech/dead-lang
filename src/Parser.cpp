#include "Parser.hpp"

std::shared_ptr<Statement>
Parser::parse(std::vector<Token> tokens, const std::shared_ptr<Supervisor>& supervisor) noexcept
{
    Parser parser(std::move(tokens), supervisor);
    return parser.parse_module();
}

Parser::Parser(std::vector<Token>&& tokens, const std::shared_ptr<Supervisor>& supervisor) noexcept
    : Iterator(tokens),
      m_supervisor{supervisor}
{
}

std::shared_ptr<Statement> Parser::parse_module() noexcept
{
    const std::string name = "main";

    std::vector<std::string>                c_includes;
    std::vector<std::shared_ptr<Statement>> structs;
    std::vector<std::shared_ptr<Statement>> functions;

    while (!eof() && !m_supervisor->has_errors()) {
        if (eol()) {
            advance(1);
            continue;
        }

        if (peek()->matches(Token::Type::C_INCLUDE)) {
            c_includes.push_back(parse_c_include_statement());
        } else if (peek()->matches(Token::Type::STRUCT)) {
            structs.push_back(parse_struct_statement());
        } else {
            functions.push_back(parse_function_statement());
        }
    }

    return std::make_shared<ModuleStatement>(
        name, c_includes, BlockStatement(structs), BlockStatement(functions));
}

std::shared_ptr<Statement> Parser::parse_function_statement() noexcept
{
    // Skip the fn token
    const auto fn_token = next();

    const auto name = next();
    if (!name || !name->matches(Token::Type::IDENTIFIER)) {
        m_supervisor->push_error(
            "expected function name after 'fn' keyword while parsing", fn_token->position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error(
            "expected '(' after function name while parsing", previous_position());
        return nullptr;
    }

    // Parse arguments
    std::vector<Typechecker::VariableDeclaration> args;
    consume_tokens_until(Token::Type::RIGHT_PAREN, [this, &args] {
        if (peek()->matches(Token::Type::COMMA)) { advance(1); }
        args.push_back(parse_variable_declaration());
    });

    // Skip the right paren
    if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
        m_supervisor->push_error(
            "expected ')' after args while parsing", fn_token->position());
        return nullptr;
    }

    // Parse return type
    std::string return_type;
    if (!matches_and_consume(Token::Type::ARROW)) {
        m_supervisor->push_error(
            "expected '->' arrow after function arguments while parsing",
            previous_position());
        return nullptr;
    }

    if (const auto ret_type = next();
        !ret_type || !ret_type->matches(Token::Type::IDENTIFIER)) {
        m_supervisor->push_error(
            "expected return type after '->' while parsing", previous_position());
        return nullptr;
    } else {
        return_type = ret_type->lexeme();
    }

    // Skip the left brace
    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error(
            "expected '{' after function return type while parsing", previous_position());
        return nullptr;
    }

    // Skip the newline
    if (!matches_and_consume(Token::Type::END_OF_LINE)) {
        m_supervisor->push_error(
            "expected newline after function return type while parsing",
            previous_position());
        return nullptr;
    }

    // Parse body
    const auto body = parse_statement_block();
    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error(
            "expected '}' after function body while parsing", previous_position());
        return nullptr;
    }

    return std::make_shared<FunctionStatement>(
        FunctionStatement(name->lexeme(), args, return_type, BlockStatement(body)));
}

std::shared_ptr<Statement> Parser::parse_statement() noexcept
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
        case Token::Type::END_OF_LINE: {
            advance(1);
            return std::make_shared<EmptyStatement>();
        }
        default: {
            return parse_expression_statement();
        }
    }
}

std::shared_ptr<Statement> Parser::parse_if_statement() noexcept
{
    // Skip the if token
    const auto if_token = next();

    // Skip the left paren
    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error(
            "expected '(' after if keyword while parsing", previous_position());
        return nullptr;
    }

    // Parse condition
    const auto condition = parse_expression();
    if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
        m_supervisor->push_error(
            "expected ')' while parsing if statement condition", previous_position());
        return nullptr;
    }

    if (!condition) {
        m_supervisor->push_error(
            "expected expression while parsing if statement condition", previous_position());
        return nullptr;
    }

    // Skip the left brace
    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error(
            "expected '{' after if condition while parsing", previous_position());
        return nullptr;
    }

    // Parse then block
    const auto then_block = parse_statement_block();

    // Skip then block right brace
    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error(
            "expected '}' after if statement's 'then branch' while parsing",
            if_token->position());
        return nullptr;
    }

    // Parse else block
    if (const auto else_token = peek();
        !else_token || !else_token->matches(Token::Type::ELSE)) {
        return std::make_shared<IfStatement>(
            IfStatement(condition, BlockStatement(then_block), BlockStatement({})));
    } else {
        const auto else_block = parse_statement_block();
        return std::make_shared<IfStatement>(IfStatement(
            condition, BlockStatement(then_block), BlockStatement(else_block)));
    }
}

std::shared_ptr<Statement> Parser::parse_return_statement() noexcept
{
    // Skip the return token
    const auto return_token = next();

    // Parse expression
    const auto expression = parse_expression();
    if (!expression) {
        m_supervisor->push_error(
            "expected expression after return keyword while parsing",
            return_token->position());
        return nullptr;
    }

    return std::make_shared<ReturnStatement>(ReturnStatement(expression));
}

std::shared_ptr<Statement> Parser::parse_variable_statement(const Token::Type& ending_delimiter) noexcept
{
    const auto variable_declaration = parse_variable_declaration();

    if (variable_declaration.type == Typechecker::BuiltinType::NONE) {
        return parse_variable_assignment();
    }

    if (Typechecker::is_fixed_size_array(variable_declaration.type_extensions)) {
        return parse_array_statement(variable_declaration);
    }

    // Skip equal sign
    const auto equal_token = peek();
    if (!matches_and_consume(Token::Type::EQUAL)) {
        m_supervisor->push_error(
            "expected '=' after variable name while parsing", previous_position());
        return nullptr;
    }

    const auto expression = parse_expression();
    if (!expression) {
        m_supervisor->push_error(
            "expected expression after '=' in variable declaration while "
            "parsing",
            equal_token->position());
        return nullptr;
    }

    if (!matches_and_consume(ending_delimiter)) {
        m_supervisor->push_error(
            "expected ';' or newline after expression in variable declaration "
            "while parsing",
            equal_token->position());
        return nullptr;
    }

    return std::make_shared<VariableStatement>(
        VariableStatement(variable_declaration, expression));
}

std::shared_ptr<Statement> Parser::parse_variable_assignment() noexcept
{
    auto variable_name = next()->lexeme();

    if (const auto next_token = peek();
        next_token && next_token->matches(Token::Type::PLUS_EQUAL)) {
        return parse_plus_equal_statement(std::move(variable_name));
    }

    m_supervisor->push_error(
        "INTERNAL ERROR: unsupported variable assignment operator", previous_position());
    return nullptr;
}

std::shared_ptr<Statement> Parser::parse_plus_equal_statement(const std::string&& variable_name) noexcept
{
    // Skip the plus_equal token
    const auto plus_equal_token = next();

    const auto expression = parse_expression();
    if (!expression) {
        m_supervisor->push_error(
            "expected expression after '+=' in variable assignment while "
            "parsing",
            plus_equal_token->position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::END_OF_LINE)) {
        m_supervisor->push_error(
            "expected ';' after expression in variable assignment while "
            "parsing",
            previous_position());
        return nullptr;
    }

    return std::make_shared<PlusEqualStatement>(PlusEqualStatement(variable_name, expression));
}

std::shared_ptr<Statement> Parser::parse_while_statement() noexcept
{
    // Skip the while token and the left paren
    const auto while_token = next();

    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error(
            "expected '(' after while keyword while parsing", previous_position());
        return nullptr;
    }

    // Parse condition
    const auto condition = parse_expression();
    // Skip the right paren
    if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
        m_supervisor->push_error(
            "expected ')' after while-loop condition while parsing",
            while_token->position());
        return nullptr;
    }

    if (!condition) {
        m_supervisor->push_error(
            "expected expression while parsing while-loop condition",
            while_token->position());
        return nullptr;
    }

    // Skip the left brace
    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error(
            "expected '{' after while-loop condition while parsing", previous_position());
        return nullptr;
    }

    // Parse body
    const auto body = parse_statement_block();

    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error(
            "expected '}' after while-loop body while parsing", previous_position());
    }

    return std::make_shared<WhileStatement>(WhileStatement(condition, BlockStatement(body)));
}

std::shared_ptr<Statement> Parser::parse_for_statement() noexcept
{
    // Skip the for token and the left paren
    const auto for_token = next();

    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error(
            "expected '(' after for keyword while parsing", previous_position());
        return nullptr;
    }

    // Parse initializer
    const auto initializer = parse_variable_statement(Token::Type::SEMICOLON);
    if (!initializer) {
        m_supervisor->push_error(
            "expected variable declaration while parsing for-loop initializer",
            for_token->position());
        return nullptr;
    }

    // Parse condition
    const auto condition = parse_expression();
    if (!condition) {
        m_supervisor->push_error(
            "expected expression while parsing for-loop condition", for_token->position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::SEMICOLON)) {
        m_supervisor->push_error(
            "expected ';' after for-loop condition while parsing", previous_position());
        return nullptr;
    }

    // Parse increment
    const auto increment = parse_expression();
    // Skip the right paren
    if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
        m_supervisor->push_error(
            "expected ')' after for-loop increment while parsing", previous_position());
        return nullptr;
    }

    if (!increment) {
        m_supervisor->push_error(
            "expected expression while parsing for-loop increment", for_token->position());
        return nullptr;
    }

    // Skip the left brace
    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error(
            "expected '{' after for-loop increment while parsing", previous_position());
        return nullptr;
    }

    // Parse body
    const auto body = parse_statement_block();

    // Skip the right brace
    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error(
            "expected '}' after for-loop body while parsing", previous_position());
    }

    return std::make_shared<ForStatement>(
        ForStatement(initializer, condition, increment, BlockStatement(body)));
}

std::shared_ptr<Statement> Parser::parse_expression_statement() noexcept
{
    const auto expression = parse_expression();
    if (!expression) {
        m_supervisor->push_error(
            "expected expression while parsing expression statement", previous_position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::END_OF_LINE)) {
        m_supervisor->push_error(
            "expected newline after expression while parsing expression "
            "statement",
            previous_position());
        return nullptr;
    }

    return std::make_shared<ExpressionStatement>(ExpressionStatement(expression));
}

std::shared_ptr<Statement>
Parser::parse_array_statement(const Typechecker::VariableDeclaration& variable_declaration) noexcept
{
    if (!matches_and_consume(Token::Type::EQUAL)) {
        m_supervisor->push_error(
            "expected '=' after array declaration while parsing", previous_position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::LEFT_BRACKET)) {
        m_supervisor->push_error(
            "expected '[' after array declaration while parsing", previous_position());
        return nullptr;
    }

    std::vector<std::shared_ptr<Expression>> array_elements;
    consume_tokens_until(Token::Type::RIGHT_BRACKET, [this, &array_elements] {
        if (peek()->matches(Token::Type::COMMA)) { advance(1); }
        const auto expression = parse_expression();
        if (!expression) { return; }
        array_elements.push_back(expression);
    });

    if (!matches_and_consume(Token::Type::RIGHT_BRACKET)) {
        m_supervisor->push_error(
            "expected ']' after array declaration while parsing", previous_position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::END_OF_LINE)) {
        m_supervisor->push_error(
            "expected newline after array declaration while parsing", previous_position());
        return nullptr;
    }

    return std::make_shared<ArrayStatement>(ArrayStatement(variable_declaration, array_elements));
}

std::shared_ptr<Expression> Parser::parse_index_operator_expression() noexcept
{
    const auto variable_name      = parse_identifier();
    const auto left_bracket_token = next();

    const auto index = parse_expression();
    if (!matches_and_consume(Token::Type::RIGHT_BRACKET)) {
        m_supervisor->push_error(
            "expected ']' after index operator while parsing", previous_position());
        return nullptr;
    }

    if (!index) {
        m_supervisor->push_error(
            "expected index in index operator while parsing",
            left_bracket_token->position());
        return nullptr;
    }

    return std::make_shared<IndexOperatorExpression>(
        IndexOperatorExpression(variable_name, index));
}

std::string Parser::parse_c_include_statement() noexcept
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

std::shared_ptr<Expression> Parser::parse_function_call_expression() noexcept
{
    const auto function_name = next()->lexeme();

    // This should never be the case since we check for this left paren when
    // entering this function, but it's always good to check + we shift the cursor by doing so.
    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error(
            "expected '(' after function name while parsing", previous_position());
        return nullptr;
    }

    std::vector<std::shared_ptr<Expression>> arguments;
    consume_tokens_until(Token::Type::RIGHT_PAREN, [this, &arguments] {
        if (peek()->matches(Token::Type::COMMA)) { advance(1); }
        arguments.push_back(parse_expression());
    });

    if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
        m_supervisor->push_error(
            "expected ')' after function call while parsing", previous_position());
        return nullptr;
    }

    return std::make_shared<FunctionCallExpression>(
        FunctionCallExpression(function_name, arguments));
}

std::shared_ptr<Statement> Parser::parse_struct_statement() noexcept
{
    const auto struct_token = next();

    const auto struct_name = parse_identifier();
    if (struct_name.empty()) { return nullptr; }

    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error(
            "expected '{' after struct name while parsing", previous_position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::END_OF_LINE)) {
        m_supervisor->push_error(
            "expected newline after '{' in struct declaration while parsing",
            previous_position());
        return nullptr;
    }

    const std::vector<Typechecker::VariableDeclaration> member_variables =
        parse_member_variables();

    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error(
            "expected '}' after struct body while parsing", previous_position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::END_OF_LINE)) {
        m_supervisor->push_error(
            "expected newline after struct declaration while parsing", previous_position());
        return nullptr;
    }

    m_defined_structs.push_back(struct_name);
    return std::make_shared<StructStatement>(StructStatement(struct_name, member_variables));
}

std::shared_ptr<Expression> Parser::parse_expression() noexcept
{
    // Binary operator
    auto lhs = parse_expression_operand();
    if (!lhs) {
        m_supervisor->push_error("expected expression while parsing", previous_position());
        return nullptr;
    }

    const auto binary_operator = peek();
    if (!binary_operator || !Token::is_binary_operator(*binary_operator)) {
        return lhs;
    }

    // Skip the binary operator token
    advance(1);

    const auto rhs = parse_expression_operand();
    if (!rhs) {
        m_supervisor->push_error(
            "expected expression right hand side operand while parsing",
            previous_position());
        return nullptr;
    }

    return std::make_shared<BinaryExpression>(
        BinaryExpression(lhs, binary_operator->type(), rhs));
}

std::shared_ptr<Expression> Parser::parse_unary_expression() noexcept
{
    const auto unary_operator = next();
    const auto operand        = parse_expression_operand();
    if (!operand) {
        m_supervisor->push_error(
            "expected operand after unary operator while parsing", previous_position());
        return nullptr;
    }

    return std::make_shared<UnaryExpression>(UnaryExpression(unary_operator->type(), operand));
}

std::shared_ptr<Expression> Parser::parse_expression_operand() noexcept
{
    if (Token::is_unary_operator(*peek())) { return parse_unary_expression(); }

    switch (peek()->type()) {
        case Token::Type::IDENTIFIER: {
            if (identifier_is_function_call()) {
                return parse_function_call_expression();
            } else if (identifier_is_index_operator()) {
                return parse_index_operator_expression();
            }
            return std::make_shared<VariableExpression>(
                VariableExpression(next()->lexeme()));
        }
        case Token::Type::SINGLE_QUOTED_STRING: {
            const auto literal = fmt::format("'{}'", next()->lexeme());
            return std::make_shared<LiteralExpression>(LiteralExpression(literal));
        }
        case Token::Type::DOUBLE_QUOTED_STRING: {
            const auto literal = fmt::format("{}", next()->lexeme());
            return std::make_shared<LiteralExpression>(LiteralExpression(literal));
        }
        case Token::Type::NUMBER: {
            return std::make_shared<LiteralExpression>(LiteralExpression(next()->lexeme()));
        }
        case Token::Type::RIGHT_PAREN:
        case Token::Type::LEFT_PAREN: {
            advance(1);
            auto expression = parse_expression();
            if (!expression) {
                m_supervisor->push_error(
                    "expected expression inside parentheses while parsing",
                    previous_position());
                return nullptr;
            }

            if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
                m_supervisor->push_error(
                    "expected ')' after expression while parsing", previous_position());
                return nullptr;
            }

            return expression;
        }
        default: {
            m_supervisor->push_error(
                fmt::format(
                    "expected expression operand while parsing, got '{}'",
                    peek()->lexeme()),
                peek()->position());
            return nullptr;
        }
    }
}

std::vector<std::shared_ptr<Statement>> Parser::parse_statement_block() noexcept
{
    std::vector<std::shared_ptr<Statement>> block;
    consume_tokens_until(Token::Type::RIGHT_BRACE, [this, &block] {
        block.push_back(parse_statement());
    });
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
        const auto variable_declaration = parse_variable_declaration();
        if (variable_declaration.type == Typechecker::BuiltinType::NONE) {
            m_supervisor->push_error(
                "unexpected variable type while parsing struct member "
                "variables",
                previous_position());
            return;
        }
        member_variables.push_back(variable_declaration);
    });

    return member_variables;
}

Typechecker::VariableDeclaration Parser::parse_variable_declaration() noexcept
{
    const bool is_mutable = peek()->type() == Token::Type::MUT;

    // Skip the mut keyword if present
    if (is_mutable) { advance(1); }

    // FIXME: This logic is garbage
    auto variable_type = Typechecker::builtin_type_from_string(peek()->lexeme());

    // Check if it is a struct type
    std::optional<std::string> custom_type = std::nullopt;
    if (is_defined_struct(*peek())) {
        variable_type = Typechecker::BuiltinType::STRUCT;
        custom_type   = peek()->lexeme();
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
        .type            = variable_type,
        .type_extensions = type_extensions,
        .is_mutable      = is_mutable,
        .name            = variable_name,
        .custom_type     = custom_type,
    };
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
    if (const auto ch = peek_ahead(1); !ch || !ch->matches(Token::Type::LEFT_PAREN)) {
        return false;
    }
    return true;
}

bool Parser::identifier_is_index_operator() const noexcept
{
    if (const auto ch = peek_ahead(1); !ch || !ch->matches(Token::Type::LEFT_BRACKET)) {
        return false;
    }
    return true;
}

bool Parser::is_defined_struct(const Token& token) const noexcept
{
    return std::ranges::find(m_defined_structs, token.lexeme()) !=
           m_defined_structs.end();
}
