#include "query_evaluator.hpp"

namespace query {

auto Evaluator::evaluate_path(const jp::JSONObject *object,
                              const query::Path &path) -> jp::expected<jp::JSONValue, Error> {
    const auto &id = path.id.identifier;

    const auto value = object->find(id);

    if (value == object->end()) {
        return Error{"Evaluator", std::format("Key '{}' not found", id), 1, 0};
    }

    if (path.next) {
        if (!value->second.is_object()) {
            return Error{"Evaluator", std::format("Key '{}' is not an object", id), 1, 0};
        }

        return evaluate_path(&std::get<jp::JSONObject>(value->second.value), **path.next);
    }

    return value->second;
}

auto Evaluator::evaluate_value(const query::Value &value) -> jp::expected<jp::JSONValue, Error> {
    return std::visit(
        overloaded{
            [&](const query::Path &path) -> jp::expected<jp::JSONValue, Error> {
                return evaluate_path(&input_json->to_object(), path);
            },
            [&](const Integer &integer) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{integer.value}; },
            [&](const Double &double_) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{double_.value}; }},
        value);
}

auto Evaluator::evaluate_expression(const query::Expression &expression) -> jp::expected<jp::JSONValue, Error> {
    // The input JSON is not an object, so we can't evaluate the expression
    if (!input_json->is_object()) {
        return *input_json;
    }

    const auto &object = std::get<jp::JSONObject>(input_json->value);

    return std::visit(
        overloaded{
            [&](const query::Value &value) -> jp::expected<jp::JSONValue, Error> { return evaluate_value(value); },
        },
        expression);
}
} // namespace query
