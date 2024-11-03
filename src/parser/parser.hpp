#pragma once

#include <span>
#include <token.hpp>
#include "error.hpp"
#include "jsonobject.hpp"
#include "expected.hpp"

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

    void push_err(Error &&err);
    void push_err(std::string &&message, unsigned line, unsigned column);
    void throw_unexpected_token(std::string &&expected, const Token &unexpected);
    void throw_unexpected_end_of_stream(std::string &&expected);
    auto get_errors() -> std::span<Error>;

  private:
    std::span<Token> tokens;
    std::vector<Error> errors;
};

auto parse(const std::string_view &json) -> expected<JSONValue, std::vector<Error>>;
} // namespace jp
