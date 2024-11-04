#pragma once
#include <cstdint>
#include <variant>
#include <memory>
#include "query_token.hpp"
#include <jsonobject.hpp>

namespace query {
struct Path {
    using NextType = std::unique_ptr<Path>;

    Identifier id;
    std::optional<std::variant<NextType, Integer>> subscript;
    std::optional<NextType> next;
};

using Value = std::variant<Path, Integer, Double>;
using Expression = std::variant<Value>;
} // namespace query
