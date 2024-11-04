#pragma once
#include <cstdint>
#include <variant>
#include <memory>
#include "query_token.hpp"
#include <jsonobject.hpp>

namespace query {

struct Function;

struct Path;
using Value = std::variant<std::unique_ptr<Path>, Integer, Double, std::unique_ptr<Function>>;

struct Path {
    using NextType = std::unique_ptr<Path>;

    Identifier id;
    std::optional<Value> subscript;
    std::optional<NextType> next;
};

using Expression = Value;

struct Function {
    Identifier name;
    std::vector<Expression> arguments;
};

} // namespace query
