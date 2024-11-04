#pragma once
#include <cstdint>
#include <variant>
#include <memory>
#include "query_token.hpp"
#include <jsonobject.hpp>

namespace query {

struct Path;
using Value = std::variant<std::unique_ptr<Path>, Integer, Double>;

struct Path {
    using NextType = std::unique_ptr<Path>;

    Identifier id;
    std::optional<Value> subscript;
    std::optional<NextType> next;
};

using Expression = std::variant<Value>;

} // namespace query
