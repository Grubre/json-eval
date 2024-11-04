#include "parser_helper.hpp"
#include "common.hpp"
#include <cmath>
#include <format>

/*
String parsing:
    string:
        '"' characters '"'

    characters:
        ""
        character characters

    character:
        '0020' . '10FFFF' - '"' - '\'
        '\' escape

    escape:
        '"'
        '\'
        '/'
        'b'
        'f'
        'n'
        'r'
        't'
        'u' hex hex hex hex

    hex:
        '0' . '9'
        'A' . 'F'
        'a' . 'f'
*/
// Expects a string_view that starts with the first character after the opening '"'
auto parse_str(std::string_view &source) -> jp::expected<std::string, Error> {
    auto current_index = 0;

    while (current_index < source.size()) {
        const auto c = source[current_index];

        if (c == '"') {
            auto result = std::string{source.substr(0, current_index)};
            source.remove_prefix(current_index + 1);
            return result;
        }

        // Verify escape characters (we save the escape character as well)
        if (c == '\\') {
            if (current_index + 1 >= source.size()) {
                return Error{"Lexer", "Unexpected end of string", 0, 0};
            }

            const auto next_c = source[current_index + 1];

            if (next_c == '"' || next_c == '\\' || next_c == '/' || next_c == 'b' || next_c == 'f' || next_c == 'n' ||
                next_c == 'r' || next_c == 't') {
                current_index += 2;
                continue;
            } else if (next_c == 'u') {
                if (current_index + 6 >= source.size()) {
                    return Error{"Lexer", "Unexpected end of string", 0, 0};
                }

                for (int i = 0; i < 4; i++) {
                    char cu = source[current_index + 2 + i];
                    if (cu == '"') {
                        return Error{"Lexer", "Expected 4 hex digits after '\\u', found closing '\"'", 0, 0};
                    }
                    if (!is_hex_digit(cu)) {
                        return Error{"Lexer", std::format("Expected 4 hex digits after '\\u', found {}", cu), 0, 0};
                    }
                }

                current_index += 6;
                continue;
            } else {
                return Error{"Lexer", std::format("Unexpected escape character '{}'", next_c), 0, 0};
            }
        }

        current_index += 1;
    }

    return Error{"Lexer", "Unexpected end of string", 0, 0};
}

auto parse_num(std::string_view &source) -> jp::expected<jp::Number, Error> {
    auto i = 0u;
    // consume digits until '.' or 'e' or 'E'
    while (i < source.size() && is_numeric(source[i])) {
        i++;
    }

    bool is_float = i < source.size() && source[i] == '.';
    // check if it's a floating point number
    if (is_float) {
        i++;
        while (i < source.size() && is_numeric(source[i])) {
            i++;
        }
    }
    const auto number_str = source.substr(0, i);

    auto value = jp::Number{};
    if (is_float) {
        value = jp::Number{.value = std::stod(std::string(number_str))};
    } else {
        value = jp::Number{.value = std::stoll(std::string(number_str))};
    }
    source.remove_prefix(i);

    // check if it's a scientific notation
    if (!source.empty() && (source[0] == 'e' || source[0] == 'E')) {
        auto i = 1u;
        if (i < source.size() && (source[i] == '+' || source[i] == '-')) {
            i++;
        }
        while (i < source.size() && is_numeric(source[i])) {
            i++;
        }

        // no number after 'e' or 'E'
        if (i == 1) {
            return Error{"Lexer", "Invalid scientific notation", 0, 0};
        }

        const auto exponential = source.substr(1, i - 1);
        const auto exp_value = std::stod(std::string(exponential));
        std::visit([&exp_value](auto &value) { value = value * std::pow(10, exp_value); }, value.value);

        source.remove_prefix(i);
    }

    return value;
}
