#include "lexer.hpp"
#include "error.hpp"
#include "expected.hpp"

#include <cmath>
#include <format>
#include <iostream>

namespace jp {

static auto is_numeric(char c) -> bool { return c >= '0' && c <= '9'; }
static auto is_lowercase_alphabetic(char c) -> bool { return (c >= 'a' && c <= 'z'); }
static auto is_uppercase_alphabetic(char c) -> bool { return (c >= 'A' && c <= 'Z'); }
static auto is_alphabetic(char c) -> bool { return is_lowercase_alphabetic(c) || is_uppercase_alphabetic(c); }
static auto is_alphanumeric(char c) -> bool { return is_alphabetic(c) || is_numeric(c); }

auto Lexer::chop(int count) -> std::string_view {
    auto result = source.substr(current_index, count);
    current_index += count;
    column_number += count;
    return result;
}

auto Lexer::chop_while(const std::function<bool(char)> &predicate) -> std::string_view {
    auto start = current_index;
    while (current_index < source.size() && predicate(source[current_index])) {
        if (source[current_index] == '\n') {
            newline();
        }

        current_index++;
        column_number++;
    }
    return source.substr(start, current_index - start);
}

auto Lexer::peek() -> std::optional<char> {
    if (current_index + 1 >= source.size()) {
        return std::nullopt;
    }

    return source[current_index + 1];
}

void Lexer::trim_whitespace() {
    while (current_index < source.size()) {
        char c = source[current_index];
        if (c == ' ' || c == '\t' || c == '\r') {
            current_index++;
            column_number++;
        } else if (c == '\n') {
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
    const auto string_begin = column_number;
    auto str = std::string{};

    // consume opening '"'
    chop(1);

    // consider \ escape sequences
    while (current_index < source.size()) {
        if (source[current_index] == '"') {
            break;
        }

        if (source[current_index] == '\\') {
            if (!peek().has_value()) {
                break;
            }
            chop(1);
            switch (source[current_index]) {
            case '"':
                str += '"';
                break;
            case '\\':
                str += '\\';
                break;
            case 'b':
                str += '\b';
                break;
            case 'f':
                str += '\f';
                break;
            case 'n':
                str += '\n';
                break;
            case 'r':
                str += '\r';
                break;
            case 't':
                str += '\t';
                break;
            /*case 'u':*/
            /*    str += "\\u";*/
            /*    break;*/
            default:
                return Error{"Lexer", std::format("Unexpected escape sequence '\\{}'", source[current_index]),
                             line_number, column_number};
            };
            chop(1);
            continue;
        }

        str += chop(1);
    }

    if (current_index >= source.size()) {
        return Error{"Lexer", "Unterminated string", line_number, column_number};
    }

    // consume ending '"'
    chop(1);
    return jp::Token{.token_type = jp::String{.value = str}, .row = line_number, .col = string_begin};
}

auto Lexer::parse_number() -> jp::expected<Token, Error> {
    const auto starting_col = column_number;

    // consume the first digit
    chop_while(is_numeric);

    // check if it's a floating point number
    if (current_index < source.size() && source[current_index] == '.') {
        chop(1);
        chop_while(is_numeric);
    }
    const auto number_str = source.substr(starting_col - 1, current_index - starting_col + 1);
    auto value = std::stod(std::string(number_str));

    // check if it's a scientific notation
    if (current_index < source.size() && (source[current_index] == 'e' || source[current_index] == 'E')) {
        chop(1);
        const auto exponential_start = current_index;
        if (current_index < source.size() && (source[current_index] == '+' || source[current_index] == '-')) {
            chop(1);
        }
        chop_while(is_numeric);

        if (exponential_start == current_index) {
            return Error{"Lexer", "Invalid scientific notation", line_number, starting_col};
        }

        const auto exponential = source.substr(exponential_start, current_index - exponential_start);
        const auto exp_value = std::stod(std::string(exponential));
        value *= std::pow(10, exp_value);
    }

    return jp::Token{.token_type = jp::Number{.value = value}, .row = line_number, .col = starting_col};
}

auto Lexer::next_token() -> std::optional<jp::expected<Token, Error>> {
    trim_whitespace();

    if (current_index >= source.size()) {
        return std::nullopt;
    }

    const char c = source[current_index];

    const auto first_char_column = column_number;

    if (is_alphabetic(c)) {
        return parse_keyword();
    }

    if (c == '"') {
        return parse_string();
    }

    // FIXME: Needs to support scientific notation
    if (is_numeric(c)) {
        return parse_number();
    }

    chop(1);
    switch (c) {
    case '{':
        return jp::Token{.token_type = jp::LBrace{}, .row = column_number, .col = first_char_column};
    case '}':
        return jp::Token{.token_type = jp::RBrace{}, .row = column_number, .col = first_char_column};
    case '[':
        return jp::Token{.token_type = jp::LBracket{}, .row = column_number, .col = first_char_column};
    case ']':
        return jp::Token{.token_type = jp::RBracket{}, .row = column_number, .col = first_char_column};
    case ',':
        return jp::Token{.token_type = jp::Comma{}, .row = column_number, .col = first_char_column};
    case ':':
        return jp::Token{.token_type = jp::Colon{}, .row = column_number, .col = first_char_column};
    };
    return Error{"Lexer", std::format("Unexpected character '{}'", c), line_number, column_number};
}
} // namespace jp
