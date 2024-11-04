#include "query_evaluator.hpp"
#include <span>

namespace query {

auto Evaluator::evaluate_path(const jp::JSONObject *object,
                              const query::Path &path) -> jp::expected<jp::JSONValue, Error> {
    const auto &id = path.id.identifier;

    const auto value = object->find(id);

    if (value == object->end()) {
        return Error{"Evaluator", std::format("Key '{}' not found", id), 1, 0};
    }

    auto evaluated = value->second;

    if (path.subscript) {
        if (!value->second.is_array()) {
            return Error{"Evaluator", std::format("Attempt to index into key '{}' which is not an array", id), 1, 0};
        }

        const auto &array = std::get<jp::JSONArray>(value->second.value);

        const auto subscript = evaluate_value(*path.subscript);

        if (!std::holds_alternative<jp::JSONInteger>(subscript->value)) {
            return Error{"Evaluator",
                         std::format("Index must be an integer, instead found {}: {}[{}]", subscript->type_str(),
                                     value->first, to_string(subscript.value())),
                         1, 0};
        }

        evaluated = array[subscript->to_integer()];
    }

    if (path.next) {
        if (!evaluated.is_object()) {
            return Error{"Evaluator", std::format("Key '{}' is not an object", id), 1, 0};
        }

        return evaluate_path(&std::get<jp::JSONObject>(evaluated.value), **path.next);
    }

    return evaluated;
}

auto Evaluator::evaluate_value(const query::Value &value) -> jp::expected<jp::JSONValue, Error> {
    return std::visit(
        overloaded{
            [&](const std::unique_ptr<Path> &path) -> jp::expected<jp::JSONValue, Error> {
                return evaluate_path(&input_json->to_object(), *path);
            },
            [&](const Integer &integer) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{integer.value}; },
            [&](const Double &double_) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{double_.value}; },
            [&](const std::unique_ptr<Function> &function) -> jp::expected<jp::JSONValue, Error> {
                return evaluate_function_call(*function);
            }},
        value);
}

void Evaluator::register_function(const std::string &name, func function) { functions[name] = std::move(function); }

auto Evaluator::evaluate_expression(const query::Expression &expression) -> jp::expected<jp::JSONValue, Error> {
    // The input JSON is not an object, so we can't evaluate the expression
    if (!input_json->is_object()) {
        return *input_json;
    }

    const auto &object = std::get<jp::JSONObject>(input_json->value);

    return evaluate_value(expression);
}

auto Evaluator::evaluate_function_call(const query::Function &function) -> jp::expected<jp::JSONValue, Error> {
    auto func = functions.find(function.name.identifier);

    if (func == functions.end()) {
        return Error{"Evaluator", std::format("Function '{}' not found", function.name.identifier), 1, 0};
    }

    return func->second(this, function.arguments);
}

} // namespace query
