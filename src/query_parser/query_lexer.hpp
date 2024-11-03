#pragma once
#include <cstdint>
#include <functional>
#include <string_view>
#include "query_token.hpp"
#include "error.hpp"
#include "expected.hpp"

namespace query {
class Lexer {
  public:
    Lexer() = delete;
    explicit Lexer(const std::string_view source) : current_index(0), source(source) {}

    auto next_token() -> std::optional<jp::expected<Token, Error>>;

  private:
    auto chop(std::uint32_t count = 1u) -> std::string_view;
    auto chop_while(const std::function<bool(char)> &predicate) -> std::string_view;
    auto peek() -> std::optional<char>;
    void trim_whitespace();

    uint32_t current_index;
    std::string_view source;
};
} // namespace query
