#include "Parser.hpp"

std::shared_ptr<Statement>
  Parser::parse(std::vector<Token> tokens, const std::shared_ptr<Supervisor>& supervisor) noexcept {
    Parser parser(std::move(tokens), supervisor);
    return parser.parse_module();
}

Parser::Parser(std::vector<Token>&& tokens, const std::shared_ptr<Supervisor>& supervisor) noexcept
  : Iterator(tokens),
    m_supervisor{ supervisor } {}

std::shared_ptr<Statement> Parser::parse_module() noexcept {
    const std::string name = "main";

    std::vector<std::shared_ptr<Statement>> functions;
    while (!eof() && !m_supervisor->has_errors()) { functions.push_back(parse_function_statement()); }

    return std::make_shared<ModuleStatement>(name, BlockStatement(functions));
}

std::shared_ptr<Statement> Parser::parse_function_statement() noexcept {
    // Skip the fn token
    const auto fn_token = next();

    const auto name = next();
    if (!name || !name->matches(Token::Type::IDENTIFIER)) {
        m_supervisor->push_error("expected function name after 'fn' keyword while parsing", previous()->position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error("expected '(' after function name while parsing", previous_position());
        return nullptr;
    }

    // Parse arguments
    std::string args;
    consume_tokens_until(Token::Type::RIGHT_PAREN, [this, &args] { args.append(" " + next()->lexeme()); });

    // Skip the right paren
    if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
        m_supervisor->push_error("expected ')' after return type while parsing", fn_token->position());
        return nullptr;
    }

    // Parse return type
    std::string return_type;
    if (!matches_and_consume(Token::Type::ARROW)) {
        m_supervisor->push_error("expected '->' arrow after function arguments while parsing", previous_position());
        return nullptr;
    }

    if (const auto ret_type = next(); !ret_type || !ret_type->matches(Token::Type::IDENTIFIER)) {
        m_supervisor->push_error("expected return type after '->' while parsing", previous_position());
        return nullptr;
    } else {
        return_type = ret_type->lexeme();
    }

    // Skip the left brace
    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error("expected '{' after function return type while parsing", previous_position());
        return nullptr;
    }

    // Parse body
    const auto body = parse_statement_block();
    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error("expected '}' after function body while parsing", previous_position());
        return nullptr;
    }

    return std::make_shared<FunctionStatement>(
      FunctionStatement(name->lexeme(), args, return_type, BlockStatement(body))
    );
}

std::shared_ptr<Statement> Parser::parse_statement() noexcept {
    switch (peek()->type()) {
        case Token::Type::IF: {
            return parse_if_statement();
        }
        case Token::Type::RETURN: {
            return parse_return_statement();
        }
        case Token::Type::MUT:
        case Token::Type::IDENTIFIER: {
            return parse_variable_statement();
        }
        case Token::Type::WHILE: {
            return parse_while_statement();
        }
        case Token::Type::FOR: {
            return parse_for_statement();
        }
        default: {
            return parse_expression_statement();
        }
    }
}

std::shared_ptr<Statement> Parser::parse_if_statement() noexcept {
    // Skip the if token
    const auto if_token = next();

    // Skip the left paren
    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error("expected '(' after if keyword while parsing", previous_position());
        return nullptr;
    }

    // Parse condition
    const auto condition = parse_expression(Token::Type::RIGHT_PAREN);
    if (!matches_and_consume(Token::Type::RIGHT_PAREN) || condition.empty()) {
        m_supervisor->push_error("expected expression while parsing if statement condition", if_token->position());
        return nullptr;
    }

    // Skip the left brace
    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error("expected '{' after if condition while parsing", previous_position());
        return nullptr;
    }

    // Parse then block
    const auto then_block = parse_statement_block();

    // Skip then block right brace
    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error("expected '}' after if statement's 'then branch' while parsing", if_token->position());
        return nullptr;
    }

    // Parse else block
    if (const auto else_token = peek(); !else_token || !else_token->matches(Token::Type::ELSE)) {
        return std::make_shared<IfStatement>(IfStatement(condition, BlockStatement(then_block), BlockStatement({})));
    } else {
        const auto else_block = parse_statement_block();
        return std::make_shared<IfStatement>(
          IfStatement(condition, BlockStatement(then_block), BlockStatement(else_block))
        );
    }
}

std::shared_ptr<Statement> Parser::parse_return_statement() noexcept {
    // Skip the return token
    const auto return_token = next();

    // Parse expression
    const auto expression = parse_expression(Token::Type::SEMICOLON);
    if (expression.empty()) {
        m_supervisor->push_error("expected expression after return keyword while parsing", previous_position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::SEMICOLON)) {
        m_supervisor->push_error(
          "expected ';' after return statement's expression while parsing", return_token->position()
        );
        return nullptr;
    }

    return std::make_shared<ReturnStatement>(ReturnStatement(expression));
}

std::shared_ptr<Statement> Parser::parse_variable_statement() noexcept {
    const bool is_mutable = peek()->type() == Token::Type::MUT;

    // Skip the mut keyword if present
    if (is_mutable) { advance(1); }

    const auto variable_type = Typechecker::builtin_type_from_string(peek()->lexeme());
    if (variable_type == Typechecker::BuiltinType::NONE) { return parse_variable_assignment(); }

    // Skip the type
    advance(1);

    std::string variable_name;
    const auto  variable_name_token = next();
    if (!variable_name_token || !variable_name_token->matches(Token::Type::IDENTIFIER)) {
        m_supervisor->push_error(
          "expected variable name after variable type while parsing", peek_behind(2)->position()
        );
        return nullptr;
    }

    variable_name = variable_name_token->lexeme();

    // Skip equal sign
    const auto equal_token = peek();
    if (!matches_and_consume(Token::Type::EQUAL)) {
        m_supervisor->push_error("expected '=' after variable name while parsing", previous_position());
        return nullptr;
    }

    // FIXME: parse_expression eats until it finds a semicolon but consider this error case:
    //      {
    //          i32 name =
    //      }
    //      return 0;
    // In this case the expression will be }return0 and it will also match the semicolon afterwards so no error;
    // What about this?
    // It will still fail because other things fail to parse, though it does not report the correct error message
    const auto expression = parse_expression(Token::Type::SEMICOLON);
    if (expression.empty()) {
        m_supervisor->push_error(
          "expected expression after '=' in variable declaration while parsing", equal_token->position()
        );
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::SEMICOLON)) {
        m_supervisor->push_error(
          "expected ';' after expression in variable declaration while parsing", equal_token->position()
        );
        return nullptr;
    }

    return std::make_shared<VariableStatement>(VariableStatement(is_mutable, variable_type, variable_name, expression));
}

std::shared_ptr<Statement> Parser::parse_variable_assignment() noexcept {
    const std::string variable_name = next()->lexeme();

    if (const auto next_token = peek(); next_token && next_token->matches(Token::Type::PLUS_EQUAL)) {
        return parse_plus_equal_statement(variable_name);
    }

    m_supervisor->push_error("INTERNAL ERROR: unsupported variable assignment operator", previous_position());
    return nullptr;
}

std::shared_ptr<Statement> Parser::parse_plus_equal_statement(const std::string& variable_name) noexcept {
    // Skip the plus_equal token
    const auto plus_equal_token = next();

    const auto expression = parse_expression(Token::Type::SEMICOLON);
    if (expression.empty()) {
        m_supervisor->push_error(
          "expected expression after '+=' in variable assignment while parsing", plus_equal_token->position()
        );
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::SEMICOLON)) {
        m_supervisor->push_error(
          "expected ';' after expression in variable assignment while parsing", previous_position()
        );
        return nullptr;
    }

    return std::make_shared<PlusEqualStatement>(PlusEqualStatement(variable_name, expression));
}

std::shared_ptr<Statement> Parser::parse_while_statement() noexcept {
    // Skip the while token and the left paren
    const auto while_token = next();

    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error("expected '(' after while keyword while parsing", previous_position());
        return nullptr;
    }

    // Parse condition
    const auto condition = parse_expression(Token::Type::RIGHT_PAREN);
    if (condition.empty()) {
        m_supervisor->push_error("expected expression while parsing while-loop condition", while_token->position());
        return nullptr;
    }

    // Skip the right paren and the left brace
    if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
        m_supervisor->push_error("expected ')' after while-loop condition while parsing", while_token->position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error("expected '{' after while-loop condition while parsing", previous_position());
        return nullptr;
    }

    // Parse body
    const auto body = parse_statement_block();

    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error("expected '}' after while-loop body while parsing", previous_position());
    }

    return std::make_shared<WhileStatement>(WhileStatement(condition, BlockStatement(body)));
}

std::shared_ptr<Statement> Parser::parse_for_statement() noexcept {
    // Skip the for token and the left paren
    const auto for_token = next();

    if (!matches_and_consume(Token::Type::LEFT_PAREN)) {
        m_supervisor->push_error("expected '(' after for keyword while parsing", previous_position());
        return nullptr;
    }

    // Parse initializer
    const auto initializer = parse_variable_statement();
    if (!initializer) {
        m_supervisor->push_error(
          "expected variable declaration while parsing for-loop initializer", for_token->position()
        );
        return nullptr;
    }

    // Parse condition
    const auto condition = parse_expression(Token::Type::SEMICOLON);

    // Skip the semicolon
    if (!matches_and_consume(Token::Type::SEMICOLON)) {
        m_supervisor->push_error("expected ';' after for-loop condition while parsing", previous_position());
        return nullptr;
    }

    // Parse increment
    const auto increment = parse_expression(Token::Type::RIGHT_PAREN);

    // Skip the right paren
    if (!matches_and_consume(Token::Type::RIGHT_PAREN)) {
        m_supervisor->push_error("expected ')' after for-loop increment while parsing", previous_position());
        return nullptr;
    }

    // Skip the left brace
    if (!matches_and_consume(Token::Type::LEFT_BRACE)) {
        m_supervisor->push_error("expected '{' after for-loop increment while parsing", previous_position());
        return nullptr;
    }

    // Parse body
    const auto body = parse_statement_block();

    // Skip the right brace
    if (!matches_and_consume(Token::Type::RIGHT_BRACE)) {
        m_supervisor->push_error("expected '}' after for-loop body while parsing", previous_position());
    }

    return std::make_shared<ForStatement>(ForStatement(initializer, condition, increment, BlockStatement(body)));
}

std::shared_ptr<Statement> Parser::parse_expression_statement() noexcept {
    const auto expression = parse_expression(Token::Type::SEMICOLON);
    if (expression.empty()) {
        m_supervisor->push_error("expected expression while parsing expression statement", previous_position());
        return nullptr;
    }

    if (!matches_and_consume(Token::Type::SEMICOLON)) {
        m_supervisor->push_error(
          "expected ';' after expression while parsing expression statement", previous_position()
        );
        return nullptr;
    }

    return std::make_shared<ExpressionStatement>(ExpressionStatement(expression));
}

std::string Parser::parse_expression(const Token::Type& delimiter) noexcept {
    std::string expression;
    consume_tokens_until(delimiter, [this, &expression] { expression.append(next()->lexeme()); });

    return expression;
}

std::vector<std::shared_ptr<Statement>> Parser::parse_statement_block() noexcept {
    std::vector<std::shared_ptr<Statement>> block;
    consume_tokens_until(Token::Type::RIGHT_BRACE, [this, &block] { block.push_back(parse_statement()); });

    return block;
}

Position Parser::previous_position() const noexcept { return previous().value_or(Token::create_dumb()).position(); }

template<std::invocable Callable>
void Parser::consume_tokens_until(const Token::Type& delimiter, Callable&& callable) noexcept {
    while (!peek()->matches(delimiter)) {
        if (eof() || m_supervisor->has_errors()) { return; }
        callable();
    }
}

bool Parser::matches_and_consume(const Token::Type& delimiter) noexcept {
    if (const auto token = peek(); !peek() || !peek()->matches(delimiter)) { return false; }

    advance(1);
    return true;
}
