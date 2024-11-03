#include <iostream>
#include <string_view>
#include "error.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

auto main() -> int {
    std::cout << "Hello World!" << std::endl;

    const std::string source = R"({"key": {"key2": "value", "key3": 123, "key4": [1, 2, 3]}})";
    auto [tokens, errors] = jp::collect_tokens(source);

    for (const auto &error : errors) {
        display_error(error);
    }

    if (!errors.empty()) {
        return 1;
    }

    auto parser = jp::Parser(tokens);
    auto obj = parser.parse();

    if (!obj) {
        for (const auto &error : parser.get_errors()) {
            display_error(error);
        }
        return 1;
    }

    auto output = jp::to_string(*obj);
    std::cout << output << std::endl;

    return 0;
}
