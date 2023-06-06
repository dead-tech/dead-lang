#include "Lexer.hpp"

std::vector<Token> Lexer::lex(std::string source, const std::shared_ptr<Supervisor>& supervisor) noexcept {
    Lexer lexer(std::move(source), supervisor);

    std::vector<Token> tokens;
    while (!lexer.eof() && !supervisor->has_errors()) {
        const auto token = lexer.next_token();
        if (!token.matches(Token::Type::END_OF_FILE)) { tokens.push_back(token); }
    }

    return tokens;
}

Lexer::Lexer(std::string&& source, const std::shared_ptr<Supervisor>& supervisor) noexcept
  : Iterator(source),
    m_supervisor{ supervisor } {}

Token Lexer::next_token() noexcept {
    if (m_supervisor->has_errors()) { return Token::create_dumb(); }

    skip_whitespaces();

    if (const auto ch = peek(); !ch.has_value()) { return Token::create_dumb(); }

    const auto ch = peek().value();
    switch (ch) {
        case '\n': {
            advance(1);
            return Token::create(Token::Type::END_OF_LINE, "\n", Position::create(cursor(), cursor()));
        }
        case '(': {
            advance(1);
            return Token::create(Token::Type::LEFT_PAREN, "(", Position::create(cursor(), cursor()));
        }
        case ')': {
            advance(1);
            return Token::create(Token::Type::RIGHT_PAREN, ")", Position::create(cursor(), cursor()));
        }
        case '-': {
            return lex_minus();
        }
        case '{': {
            advance(1);
            return Token::create(Token::Type::LEFT_BRACE, "{", Position::create(cursor(), cursor()));
        }
        case '}': {
            advance(1);
            return Token::create(Token::Type::RIGHT_BRACE, "}", Position::create(cursor(), cursor()));
        }
        case '=': {
            return lex_equal_sign();
        }
        case ';': {
            advance(1);
            return Token::create(Token::Type::SEMICOLON, ";", Position::create(cursor(), cursor()));
        }
        case '*': {
            return lex_star();
        }
        case ',': {
            advance(1);
            return Token::create(Token::Type::COMMA, ",", Position::create(cursor(), cursor()));
        }
        case '&': {
            advance(1);
            return Token::create(Token::Type::AMPERSAND, "&", Position::create(cursor(), cursor()));
        }
        case '[': {
            advance(1);
            return Token::create(Token::Type::LEFT_BRACKET, "[", Position::create(cursor(), cursor()));
        }
        case ']': {
            advance(1);
            return Token::create(Token::Type::RIGHT_BRACKET, "]", Position::create(cursor(), cursor()));
        }
        case '+': {
            return lex_plus();
        }
        case '<': {
            return lex_less_than();
        }
        case '\'': {
            return lex_single_quoted_string();
        }
        case '"': {
            return lex_double_quoted_string();
        }
        default: {
            return lex_keyword_or_identifier();
        }
    }
}

void Lexer::skip_whitespaces() noexcept {
    consume_chars([this](const auto& ch) {
        if (ch == ' ' || ch == '\t' || ch == '\r') {
            advance(1);
            return dts::IteratorDecision::Continue;
        } else {
            return dts::IteratorDecision::Break;
        }
    });
}

Token Lexer::lex_keyword_or_identifier() noexcept {
    const auto start = cursor();

    if (std::isdigit(peek().value()) != 0) { return lex_number(); }

    std::string value;
    consume_chars([this, &value](const auto& ch) {
        if (std::isalnum(ch) != 0 || ch == '_') {
            value += ch;
            advance(1);
            return dts::IteratorDecision::Continue;
        } else {
            return dts::IteratorDecision::Break;
        }
    });

    if (const auto keyword = Token::is_keyword(value); keyword.has_value()) {
        return Token::create(*keyword, std::move(value), Position::create(start, cursor()));
    }

    return Token::create(Token::Type::IDENTIFIER, std::move(value), Position::create(start, cursor()));
}

Token Lexer::lex_minus() noexcept {
    const auto start = cursor();

    if (auto ch = peek_ahead(1); ch == '>') {
        advance(2);
        return Token::create(Token::Type::ARROW, "->", Position::create(start, cursor()));
    } else if (ch = peek_ahead(1); ch == '-') {
        advance(2);
        return Token::create(Token::Type::MINUS_MINUS, "--", Position::create(start, cursor()));
    }

    advance(1);
    return Token::create(Token::Type::MINUS, "-", Position::create(start, cursor()));
}

Token Lexer::lex_equal_sign() noexcept {
    const auto start = cursor();

    if (auto ch = peek_ahead(1); ch == '=') {
        advance(2);
        return Token::create(Token::Type::EQUAL_EQUAL, "==", Position::create(start, cursor()));
    }

    advance(1);
    return Token::create(Token::Type::EQUAL, "=", Position::create(start, cursor()));
}

Token Lexer::lex_star() noexcept {
    const auto start = cursor();
    advance(1);
    return Token::create(Token::Type::STAR, "*", Position::create(start, cursor()));
}

Token Lexer::lex_plus() noexcept {
    const auto start = cursor();

    if (auto ch = peek_ahead(1); ch == '=') {
        advance(2);
        return Token::create(Token::Type::PLUS_EQUAL, "+=", Position::create(start, cursor()));
    }

    advance(1);
    return Token::create(Token::Type::PLUS, "+", Position::create(start, cursor()));
}

Token Lexer::lex_less_than() noexcept {
    const auto start = cursor();

    if (auto ch = peek_ahead(1); ch == '=') {
        advance(2);
        return Token::create(Token::Type::LESS_EQUAL, "<=", Position::create(start, cursor()));
    }

    advance(1);
    return Token::create(Token::Type::LESS, "<", Position::create(start, cursor()));
}

Token Lexer::lex_single_quoted_string() noexcept {
    const auto start = cursor();

    // Skip the opening single quote
    advance(1);
    const auto quoted       = next();
    const auto ending_quote = next();

    if (!quoted || !ending_quote || ending_quote.value() != '\'') {
        m_supervisor->push_error("unterminated or empty single quoted string", Position::create(start, cursor()));
        return Token::create_dumb();
    }

    return Token::create(
      Token::Type::SINGLE_QUOTED_STRING, std::string(1, quoted.value()), Position::create(start, cursor())
    );
}

Token Lexer::lex_number() noexcept {
    const auto start = cursor();

    std::string value;
    consume_chars([this, &value](const auto& ch) {
        if (std::isdigit(ch) != 0) {
            value += ch;
            advance(1);
            return dts::IteratorDecision::Continue;
        } else {
            return dts::IteratorDecision::Break;
        }
    });

    return Token::create(Token::Type::NUMBER, std::move(value), Position::create(start, cursor()));
}

Token Lexer::lex_double_quoted_string() noexcept {
    const auto start = cursor();

    // Skip the opening double quote
    advance(1);

    std::string value;
    consume_chars([this, &value](const auto& ch) {
        if (ch != '"') {
            value += ch;
            advance(1);
            return dts::IteratorDecision::Continue;
        } else if (ch == '\n') {
            m_supervisor->push_error("unterminated double quoted string", Position::create(cursor(), cursor()));
            return dts::IteratorDecision::Break;
        }

        return dts::IteratorDecision::Break;
    });

    // Skip the ending double quote
    advance(1);

    return Token::create(Token::Type::DOUBLE_QUOTED_STRING, std::move(value), Position::create(start, cursor()));
}

template<std::invocable<char> Callable>
void Lexer::consume_chars(Callable&& callable) noexcept {
    while (!eof()) {
        const auto iterator_decision = callable(*peek());
        if (iterator_decision == dts::IteratorDecision::Break) {
            break;
        } else if (iterator_decision == dts::IteratorDecision::Continue) {
            continue;
        }
    }
}
