#include "query_lexer.hpp"
#include "parser_helper.hpp"
#include "common.hpp"

namespace query {

auto Lexer::chop() -> char {
    auto result = source.front();
    source.remove_prefix(1);
    column_number++;
    return result;
}

auto Lexer::chop_while(const std::function<bool(char)> &predicate) -> std::string_view {
    auto i = 0u;
    while (i < source.size() && predicate(source[i])) {
        i++;
        column_number++;
    }

    auto result = source.substr(0, i);
    source.remove_prefix(i);
    return result;
}

auto Lexer::peek() -> std::optional<char> {
    if (source.empty()) {
        return std::nullopt;
    }
    return source.front();
}

void Lexer::trim_whitespace() {
    while (auto c = peek()) {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
        chop();
    }
}

auto Lexer::next_token() -> std::optional<jp::expected<Token, Error>> {
    trim_whitespace();

    const auto first_char_column = column_number;

    if (source.empty()) {
        return std::nullopt;
    }

    const char c = *peek();

    if (is_numeric(c)) {
        auto number = chop_while(is_numeric);
        return Token{Integer{std::stoll(std::string{number})}, first_char_column};
    }

    if (is_alphabetic(c)) {
        auto identifier = chop_while(is_alphabetic);
        return Token{Identifier{std::string{identifier}}, first_char_column};
    }

    chop();

    if (c == '.') {
        return Token{Dot{}, first_char_column};
    } else if (c == ',') {
        return Token{Comma{}, first_char_column};
    } else if (c == '[') {
        return Token{LBracket{}, first_char_column};
    } else if (c == ']') {
        return Token{RBracket{}, first_char_column};
    } else if (c == '(') {
        return Token{LParen{}, first_char_column};
    } else if (c == ')') {
        return Token{RParen{}, first_char_column};
    } else {
        return Error{
            .source = "Query Lexer", .message = "Unexpected character", .line = 0, .column = first_char_column};
    }
}

auto collect_tokens(const std::string_view source) -> std::pair<std::vector<Token>, std::vector<Error>> {
    auto tokens = std::vector<Token>{};
    auto errors = std::vector<Error>{};

    auto lexer = Lexer{source};

    while (auto next_token = lexer.next_token()) {
        if (next_token->has_value()) {
            tokens.push_back(next_token->value());
        } else {
            errors.push_back(next_token->error());
        }
    }
    return {tokens, errors};
}

} // namespace query
