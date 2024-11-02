#include <iostream>
#include <string_view>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

auto main() -> int {
    std::cout << "Hello World!" << std::endl;

    const std::string source = R"({"key": "abc", "key2": 123})";
    auto [tokens, errors] = jp::collect_tokens(source);

    auto parser = jp::Parser(tokens);
    auto obj = parser.parse_object();

    if (!obj) {
        std::cerr << "Failed to parse object" << std::endl;
        return 1;
    }

    for (const auto &[key, value] : *obj) {
        std::cout << key << ": ...";
    }

    return 0;
}
