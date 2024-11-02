#pragma once

#include <span>
#include <token.hpp>
#include "jsonobject.hpp"

namespace jp {

class Parser {
  public:
    Parser() = delete;
    explicit Parser(const std::span<jp::Token> &tokens) : tokens(tokens) {}

    auto chop() -> std::optional<jp::Token>;
    [[nodiscard]] auto peek() const -> const jp::Token *;
    auto parse() -> std::optional<JSONValue>;
    auto parse_object() -> std::optional<JSONObject>;
    auto parse_array() -> std::optional<JSONArray>;
    auto parse_value() -> std::optional<JSONValue>;

  private:
    std::span<Token> tokens;
};
} // namespace jp
