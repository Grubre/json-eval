#include "lexer.hpp"
#include "common.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "parser_helper.hpp"

#include <cmath>
#include <format>
#include <iomanip>
#include <iostream>

namespace jp {

auto Lexer::chop() -> char {
    auto result = source.front();
    source.remove_prefix(1);
    column_number++;
    return result;
}

auto Lexer::chop_while(const std::function<bool(char)> &predicate) -> std::string_view {
    auto i = 0u;
    while (i < source.size() && predicate(source[i])) {
        if (source[i] == '\n') {
            newline();
        }

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
        if (c == ' ' || c == '\t' || c == '\r') {
            chop();
        } else if (c == '\n') {
            chop();
            newline();
        } else {
            break;
        }
    }
}

void Lexer::newline() {
    line_number++;
    column_number = 1;
}

auto Lexer::parse_keyword() -> jp::expected<Token, Error> {
    auto keyword = chop_while(is_alphabetic);

    if (keyword == "true") {
        return jp::Token{.token_type = jp::True{}, .row = line_number, .col = column_number};
    }

    if (keyword == "false") {
        return jp::Token{.token_type = jp::False{}, .row = line_number, .col = column_number};
    }

    if (keyword == "null") {
        return jp::Token{.token_type = jp::Null{}, .row = line_number, .col = column_number};
    }

    return Error{"Lexer", std::format("Unexpected keyword '{}'", keyword), line_number, column_number};
}

auto Lexer::parse_string() -> jp::expected<Token, Error> {
    const auto starting_col = column_number;
    chop(); // consume opening quote
    const auto source_size_before = source.size();
    auto str = parse_str(source);
    if (str.has_error()) {
        auto error = str.consume_error();
        error.column = starting_col;
        error.line = line_number;
        return error;
    }
    column_number += source_size_before - source.size();
    return jp::Token{
        .token_type = jp::String{.value = std::move(str.consume_value())}, .row = line_number, .col = starting_col};
}

auto Lexer::parse_number() -> jp::expected<Token, Error> {
    const auto starting_col = column_number;
    const auto source_size_before = source.size();
    auto number = parse_num(source);
    if (number.has_error()) {
        auto error = number.consume_error();
        error.column = starting_col;
        error.line = line_number;
        return error;
    }
    column_number += source_size_before - source.size();
    return Token{number.consume_value(), line_number, starting_col};
}

auto Lexer::next_token() -> std::optional<jp::expected<Token, Error>> {
    trim_whitespace();

    const auto first_char_column = column_number;

    if (source.empty()) {
        return std::nullopt;
    }

    const char c = *peek();

    if (is_numeric(c)) {
        return parse_number();
    }

    if (is_alphabetic(c)) {
        return parse_keyword();
    }

    if (c == '"') {
        return parse_string();
    }

    chop();

    switch (c) {
    case '{':
        return jp::Token{.token_type = jp::LBrace{}, .row = line_number, .col = first_char_column};
    case '}':
        return jp::Token{.token_type = jp::RBrace{}, .row = line_number, .col = first_char_column};
    case '[':
        return jp::Token{.token_type = jp::LBracket{}, .row = line_number, .col = first_char_column};
    case ']':
        return jp::Token{.token_type = jp::RBracket{}, .row = line_number, .col = first_char_column};
    case ',':
        return jp::Token{.token_type = jp::Comma{}, .row = line_number, .col = first_char_column};
    case ':':
        return jp::Token{.token_type = jp::Colon{}, .row = line_number, .col = first_char_column};
    };
    return Error{"Lexer", std::format("Unexpected character '{}'", c), line_number, first_char_column};
}

auto collect_tokens(const std::string_view source) -> std::pair<std::vector<Token>, std::vector<Error>> {
    auto tokens = std::vector<Token>{};
    auto errors = std::vector<Error>{};

    for (const auto &next_token : Lexer(source)) {
        if (next_token.has_value()) {
            tokens.push_back(next_token.value());
        } else {
            errors.push_back(next_token.error());
        }
    }

    return {tokens, errors};
}

} // namespace jp
