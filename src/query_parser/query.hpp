#pragma once
#include <cstdint>
#include <variant>
#include <memory>
#include "query_token.hpp"
#include <jsonobject.hpp>

namespace query {

struct Function;
struct Unary;
struct Binary;

struct Path;
using Value = std::variant<std::unique_ptr<Path>, Integer, Double, std::unique_ptr<Function>, std::unique_ptr<Binary>,
                           std::unique_ptr<Unary>>;

struct Path {
    using NextType = std::unique_ptr<Path>;

    Identifier id;
    std::optional<Value> subscript;
    std::optional<NextType> next;
};

using Expression = Value;

struct Unary {
    Token op;
    Value value;
};

struct Binary {
    Token op;
    Value lhs;
    Value rhs;
};

struct Function {
    Identifier name;
    std::vector<Expression> arguments;
};

} // namespace query
