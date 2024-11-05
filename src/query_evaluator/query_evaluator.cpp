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

        evaluated = array[subscript->as_integer()];
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
                return evaluate_path(&input_json->as_object(), *path);
            },
            [&](const Integer &integer) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{integer.value}; },
            [&](const Double &double_) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{double_.value}; },
            [&](const std::unique_ptr<Function> &function) -> jp::expected<jp::JSONValue, Error> {
                return evaluate_function_call(*function);
            },
            [&](const std::unique_ptr<Binary> &binary) -> jp::expected<jp::JSONValue, Error> {
                return evaluate_binary(*binary);
            },
            [&](const std::unique_ptr<Unary> &unary) -> jp::expected<jp::JSONValue, Error> {
                return evaluate_unary(*unary);
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

auto Evaluator::evaluate_binary(const query::Binary &binary) -> jp::expected<jp::JSONValue, Error> {
    auto lhs = evaluate_value(binary.lhs);
    if (lhs.has_error()) {
        return lhs;
    }

    auto rhs = evaluate_value(binary.rhs);
    if (rhs.has_error()) {
        return rhs;
    }

    if (!lhs->is_numeric() || !rhs->is_numeric()) {
        return Error{"Evaluator",
                     std::format("Unsupported binary operation on types: {} and {}", lhs->type_str(), rhs->type_str()),
                     1, 0};
    }

    auto lhs_num = lhs->to_double();
    auto rhs_num = rhs->to_double();

    return std::visit(
        overloaded{
            [&](const Plus &) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{lhs_num + rhs_num}; },
            [&](const Minus &) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{lhs_num - rhs_num}; },
            [&](const Star &) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{lhs_num * rhs_num}; },
            [&](const Slash &) -> jp::expected<jp::JSONValue, Error> {
                if (rhs_num == 0) {
                    return Error{"Evaluator", "Division by zero", 1, 0};
                }
                return jp::JSONValue{lhs_num / rhs_num};
            },
            [&](const auto &token) -> jp::expected<jp::JSONValue, Error> {
                return Error{"Evaluator", std::format("Unsupported binary operation: {}", to_string(token)), 1, 0};
            },
        },
        binary.op.token_type);

    return Error{"Evaluator", std::format("Unsupported binary operation on type: {}", lhs->type_str()), 1, 0};
}

auto Evaluator::evaluate_unary(const query::Unary &unary) -> jp::expected<jp::JSONValue, Error> {
    auto value = evaluate_value(unary.value);

    if (!value->is_numeric()) {
        return Error{"Evaluator", std::format("Unsupported unary operation on type: {}", value->type_str()), 1, 0};
    }

    auto num = value->to_double();

    return std::visit(overloaded{
                          [&](const Minus &) -> jp::expected<jp::JSONValue, Error> { return jp::JSONValue{-num}; },
                          [&](const auto &token) -> jp::expected<jp::JSONValue, Error> {
                              return Error{"Evaluator",
                                           std::format("Unsupported unary operation: {}", to_string(token)), 1, 0};
                          },
                      },
                      unary.op.token_type);

    return Error{"Evaluator", std::format("Unsupported unary operation on type: {}", value->type_str()), 1, 0};
}

} // namespace query
