#pragma once
#include <vector>
#include "error.hpp"
#include "query.hpp"
#include "query_token.hpp"

namespace query {

class Parser {
  public:
    Parser() = delete;
    explicit Parser(const std::span<query::Token> &tokens) : tokens(tokens) {}

    auto chop() -> std::optional<query::Token>;
    [[nodiscard]] auto peek() const -> const query::Token *;

    auto parse() -> std::optional<query::Expression>;
    auto parse_expression() -> std::optional<query::Expression>;
    auto parse_value() -> std::optional<query::Value>;

    void push_err(Error &&err);
    void push_err(std::string &&message, unsigned column);
    void throw_unexpected_token(std::string &&expected, const Token &unexpected);
    void throw_unexpected_end_of_stream(std::string &&expected);
    auto get_errors() -> std::span<Error>;

  private:
    std::span<Token> tokens;
    std::vector<Error> errors;
};

} // namespace query
