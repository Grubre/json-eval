#include <iostream>
#include <string_view>
#include "error.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "query_token.hpp"

auto main() -> int {
    std::cout << "Hello World!" << std::endl;

    const std::string source = R"(123)";
    auto [tokens, errors] = jp::collect_tokens(source);

    for (const auto &token : tokens) {
        if (std::holds_alternative<jp::Number>(token.token_type)) {
            auto number = std::get<jp::Number>(token.token_type);
            if (std::holds_alternative<int64_t>(number.value)) {
                std::cout << "integer: " << std::get<int64_t>(number.value) << std::endl;
            } else if (std::holds_alternative<double>(number.value)) {
                std::cout << "double: " << std::get<double>(number.value) << std::endl;
            }
        }
        std::cout << "token:" << token.row << ":" << token.col << ": " << to_string(token.token_type) << std::endl;
    }

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
