#pragma once

#include "error.hpp"
#include "expected.hpp"
#include "jsonobject.hpp"
#include "query.hpp"
namespace query {

class Evaluator {
  public:
    explicit Evaluator(const jp::JSONValue *input_json) : input_json(input_json) {}

    auto evaluate_expression(const query::Expression &expression) -> jp::expected<jp::JSONValue, Error>;

  private:
    auto evaluate_value(const query::Value &value) -> jp::expected<jp::JSONValue, Error>;
    auto evaluate_path(const jp::JSONObject *object, const query::Path &path) -> jp::expected<jp::JSONValue, Error>;

    const jp::JSONValue *input_json;
};

} // namespace query
