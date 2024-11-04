#include <iostream>
#include <string_view>
#include "error.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "query_parser.hpp"
#include "query_lexer.hpp"
#include "query_evaluator.hpp"

void register_intrinsic_functions(query::Evaluator &evaluator);

auto main() -> int {
    std::cout << "Hello World!" << std::endl;

    const std::string source = R"({ "array": [1,[2],3], "b": { "c": 1 } })";
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

    const std::string query = R"(size(array))";

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

    register_intrinsic_functions(evaluator);

    auto result = evaluator.evaluate_expression(*expression);

    if (!result.has_value()) {
        display_error(result.error());
        return 1;
    }

    std::cout << "Result: " << jp::to_string(result.value()) << std::endl;

    return 0;
}

void register_intrinsic_functions(query::Evaluator &evaluator) {
    evaluator.register_function(
        "size",
        [](query::Evaluator *evaluator,
           const std::span<const query::Expression> &args) -> jp::expected<jp::JSONValue, Error> {
            if (args.size() != 1) {
                return Error{"Evaluator", "size() expects exactly 1 argument", 1, 0};
            }

            const auto &arg = args[0];
            const auto result = evaluator->evaluate_expression(arg);

            if (!result.has_value()) {
                return result.error();
            }

            if (!result->is_array() && !result->is_object()) {
                return Error{
                    "Evaluator",
                    std::format("size() expects an array as its argument, instead found {}", result->type_str()), 1, 0};
            }

            if (result->is_array()) {
                return jp::JSONValue{(jp::JSONInteger)std::get<jp::JSONArray>(result->value).size()};
            }

            return jp::JSONValue{(jp::JSONInteger)std::get<jp::JSONObject>(result->value).size()};
        });
}
