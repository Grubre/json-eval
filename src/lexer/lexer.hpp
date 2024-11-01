#pragma once

#include "error.hpp"
#include "expected.hpp"
#include "token.hpp"
#include <functional>
#include <optional>
#include <string>
#include <iterator>

namespace jp {

class Lexer {
  public:
    Lexer() = delete;
    explicit Lexer(const std::string_view source)
        : line_number(1), column_number(1), current_index(0), source(source) {}

    auto next_token() -> std::optional<jp::expected<Token, Error>>;

  private:
    auto chop(int count) -> std::string_view;
    auto chop_while(const std::function<bool(char)> &predicate) -> std::string_view;
    auto peek() -> std::optional<char>;
    void trim_whitespace();
    void newline();

    auto parse_keyword() -> jp::expected<Token, Error>;
    auto parse_string() -> jp::expected<Token, Error>;
    auto parse_number() -> jp::expected<Token, Error>;

    unsigned line_number;
    unsigned column_number;
    unsigned current_index;
    std::string_view source;
};

} // namespace jp
