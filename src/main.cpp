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

    const std::string source = R"({ "array": [1,2,3,4,5] })";
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

    const std::string query = R"(max(array))";

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
                return Error{"Evaluator",
                             std::format("size() expects an array or object as its argument, instead found {}",
                                         result->type_str()),
                             1, 0};
            }

            if (result->is_array()) {
                return jp::JSONValue{static_cast<jp::JSONInteger>(std::get<jp::JSONArray>(result->value).size())};
            }

            return jp::JSONValue{static_cast<jp::JSONInteger>(std::get<jp::JSONObject>(result->value).size())};
        });

    evaluator.register_function(
        "max",
        [](query::Evaluator *evaluator,
           const std::span<const query::Expression> &args) -> jp::expected<jp::JSONValue, Error> {
            if (args.empty()) {
                return Error{"Evaluator", "max() expects at least one argument", 1, 0};
            }

            auto evaluate_and_check_number = [&](const query::Expression &arg) -> jp::expected<jp::JSONDouble, Error> {
                const auto result = evaluator->evaluate_expression(arg);

                if (!result.has_value()) {
                    return result.error();
                }

                if (result->is_integer()) {
                    return static_cast<jp::JSONDouble>(std::get<jp::JSONInteger>(result->value));
                } else if (result->is_double()) {
                    return std::get<jp::JSONDouble>(result->value);
                } else {
                    return Error{"Evaluator",
                                 std::format("max() expects numbers or an array of numbers, instead found {}",
                                             result->type_str()),
                                 1, 0};
                }
            };

            // Single argument case: If it's an array, find max in array; otherwise, treat args as list of numbers.
            if (args.size() == 1) {
                const auto result = evaluator->evaluate_expression(args[0]);

                if (!result.has_value()) {
                    return result.error();
                }

                if (result->is_array()) {
                    const auto &array = std::get<jp::JSONArray>(result->value);
                    if (array.empty()) {
                        return Error{"Evaluator", "max() received an empty array", 1, 0};
                    }

                    jp::JSONDouble max_value = std::numeric_limits<jp::JSONDouble>::lowest();
                    for (const auto &item : array) {
                        if (item.is_integer()) {
                            max_value =
                                std::max(max_value, static_cast<jp::JSONDouble>(std::get<jp::JSONInteger>(item.value)));
                        } else if (item.is_double()) {
                            max_value = std::max(max_value, std::get<jp::JSONDouble>(item.value));
                        } else {
                            return Error{"Evaluator", "max() expects array elements to be numbers", 1, 0};
                        }
                    }
                    return jp::JSONValue{max_value};
                }
                // Fall through for single non-array argument, handled as a list of one.
            }

            // Multi-argument case or single non-array argument
            jp::JSONDouble max_value = std::numeric_limits<jp::JSONDouble>::lowest();
            for (const auto &arg : args) {
                auto value = evaluate_and_check_number(arg);
                if (!value.has_value()) {
                    return value.error();
                }
                max_value = std::max(max_value, *value);
            }

            return jp::JSONValue{max_value};
        });
}
