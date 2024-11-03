#include "query_lexer.hpp"
#include "common.hpp"

namespace query {

auto Lexer::next_token() -> std::optional<jp::expected<Token, Error>> {
    trim_whitespace();

    if (current_index >= source.size()) {
        return std::nullopt;
    }

    char c = source[current_index];
    if (is_numeric(c)) {
        auto number = chop_while(is_numeric);
        return Token{Number{std::stoull(std::string{number})}, current_index};
    } else if (is_alphabetic(c)) {
        auto identifier = chop_while(is_alphanumeric);
        return Token{Identifier{std::string{identifier}}, current_index};
    } else if (c == '.') {
        current_index++;
        return Token{Dot{}, current_index};
    } else if (c == ',') {
        current_index++;
        return Token{Comma{}, current_index};
    } else if (c == '[') {
        current_index++;
        return Token{LBracket{}, current_index};
    } else if (c == ']') {
        current_index++;
        return Token{RBracket{}, current_index};
    } else {
        return Error{.source = "Query Lexer", .message = "Unexpected character", .line = 0, .column = current_index};
    }
}
} // namespace query
