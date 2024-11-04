#include <iostream>
#include <string_view>
#include "error.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "query_parser.hpp"
#include "query_lexer.hpp"
#include "query_evaluator.hpp"

auto main() -> int {
    std::cout << "Hello World!" << std::endl;

    const std::string source = R"({"a": { "b": [ 1, 2, { "c": "test" }, [11, 12] ]}})";
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
    std::cout << "Input JSON: " << output << std::endl;

    const std::string query = R"(a.b[a.b[1]].c)";

    auto [query_tokens, query_errors] = query::collect_tokens(query);

    for (const auto &error : query_errors) {
        display_error(error);
    }

    if (!query_errors.empty()) {
        return 1;
    }

    auto query_parser = query::Parser(query_tokens);

    auto expression = query_parser.parse();

    if (!expression) {
        for (const auto &error : query_parser.get_errors()) {
            display_error(error);
        }
        return 1;
    }

    auto evaluator = query::Evaluator(&*obj);

    auto result = evaluator.evaluate_expression(*expression);

    if (!result.has_value()) {
        display_error(result.error());
        return 1;
    }

    std::cout << "Result: " << jp::to_string(result.value()) << std::endl;

    return 0;
}
