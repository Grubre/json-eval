#include <iostream>
#include <fstream>
#include <filesystem>
#include "error.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "query_parser.hpp"
#include "query_lexer.hpp"
#include "query_evaluator.hpp"

void register_intrinsic_functions(query::Evaluator &evaluator);

auto main(int argc, char *argv[]) -> int {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <path_to_json> <query>" << std::endl;
        return 1;
    }

    const std::string path = argv[1];
    const std::string query = argv[2];

    if (!std::filesystem::exists(path)) {
        std::cerr << "File does not exist: " << path << std::endl;
        return 1;
    }

    auto file = std::ifstream{path};
    auto source = std::string{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};

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

    if (query.empty()) {
        std::cout << jp::to_string(*obj) << std::endl;
        return 0;
    }

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

    std::cout << jp::to_string(result.value()) << std::endl;

    return 0;
}

void register_list_function(query::Evaluator &evaluator, const std::string &name,
                            const std::function<jp::JSONDouble(jp::JSONDouble, jp::JSONDouble)> &compare_fn,
                            jp::JSONDouble default_val) {
    evaluator.register_function(
        name,
        [compare_fn, default_val, name](query::Evaluator *evaluator, const std::span<const query::Expression> &args)
            -> jp::expected<jp::JSONValue, Error> {
            if (args.empty()) {
                return Error{"Evaluator", name + "() expects at least one argument", 1, 0};
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
                                 std::format("{}() expects numbers or an array of numbers, instead found {}", name,
                                             result->type_str()),
                                 1, 0};
                }
            };

            // Single argument case: If it's an array, apply compare_fn on array; otherwise, treat args as list of
            // numbers.
            if (args.size() == 1) {
                const auto result = evaluator->evaluate_expression(args[0]);

                if (!result.has_value()) {
                    return result.error();
                }

                if (result->is_array()) {
                    const auto &array = std::get<jp::JSONArray>(result->value);
                    if (array.empty()) {
                        return Error{"Evaluator", name + "() received an empty array", 1, 0};
                    }

                    jp::JSONDouble result_value = default_val;
                    for (const auto &item : array) {
                        if (item.is_integer()) {
                            result_value = compare_fn(
                                result_value, static_cast<jp::JSONDouble>(std::get<jp::JSONInteger>(item.value)));
                        } else if (item.is_double()) {
                            result_value = compare_fn(result_value, std::get<jp::JSONDouble>(item.value));
                        } else {
                            return Error{"Evaluator", name + "() expects array elements to be numbers", 1, 0};
                        }
                    }
                    return jp::JSONValue{result_value};
                }
                // Fall through for single non-array argument, handled as a list of one.
            }

            // Multi-argument case or single non-array argument
            jp::JSONDouble result_value = default_val;
            for (const auto &arg : args) {
                auto value = evaluate_and_check_number(arg);
                if (!value.has_value()) {
                    return value.error();
                }
                result_value = compare_fn(result_value, *value);
            }

            return jp::JSONValue{result_value};
        });
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

    register_list_function(
        evaluator, "max", [](jp::JSONDouble a, jp::JSONDouble b) { return std::max(a, b); },
        std::numeric_limits<jp::JSONDouble>::lowest());
    register_list_function(
        evaluator, "min", [](jp::JSONDouble a, jp::JSONDouble b) { return std::min(a, b); },
        std::numeric_limits<jp::JSONDouble>::max());
    register_list_function(evaluator, "sum", [](jp::JSONDouble a, jp::JSONDouble b) { return a + b; }, 0);
    register_list_function(evaluator, "product", [](jp::JSONDouble a, jp::JSONDouble b) { return a * b; }, 1);
}
