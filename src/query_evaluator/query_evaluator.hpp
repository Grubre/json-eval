#pragma once

#include "error.hpp"
#include "expected.hpp"
#include "jsonobject.hpp"
#include "query.hpp"
#include <functional>

namespace query {

class Evaluator {
  public:
    using func = std::function<jp::expected<jp::JSONValue, Error>(Evaluator *, const std::span<const Expression> &)>;
    explicit Evaluator(const jp::JSONValue *input_json) : input_json(input_json) {}

    auto evaluate_expression(const query::Expression &expression) -> jp::expected<jp::JSONValue, Error>;
    auto evaluate_value(const query::Value &value) -> jp::expected<jp::JSONValue, Error>;
    auto evaluate_function_call(const query::Function &function) -> jp::expected<jp::JSONValue, Error>;
    auto evaluate_path(const jp::JSONObject *object, const query::Path &path) -> jp::expected<jp::JSONValue, Error>;
    auto evaluate_binary(const query::Binary &binary) -> jp::expected<jp::JSONValue, Error>;
    auto evaluate_unary(const query::Unary &unary) -> jp::expected<jp::JSONValue, Error>;
    void register_function(const std::string &name, func function);

    std::unordered_map<std::string, func> functions;
    const jp::JSONValue *input_json;
};

} // namespace query
