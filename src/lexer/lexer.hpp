#pragma once

#include "error.hpp"
#include "expected.hpp"
#include "token.hpp"
#include <functional>
#include <optional>
#include <iterator>

namespace jp {

class Lexer {
  public:
    Lexer() = delete;
    explicit Lexer(const std::string_view source)
        : line_number(1), column_number(1), current_index(0), source(source) {}

    auto next_token() -> std::optional<jp::expected<Token, Error>>;

    class Iterator {
      public:
        using iterator_category = std::input_iterator_tag;
        using value_type = jp::expected<Token, Error>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type *;
        using reference = const value_type &;

        explicit Iterator(Lexer *lexer) : current_token{lexer->next_token()}, lexer{current_token ? lexer : nullptr} {}
        Iterator() = default;

        auto operator*() const -> reference { return *current_token; }
        auto operator->() const -> pointer { return &(*current_token); }

        auto operator++() -> Iterator & {
            if (lexer != nullptr) {
                current_token = lexer->next_token();
                if (!current_token) {
                    lexer = nullptr;
                }
            }
            return *this;
        }

        auto operator==(const Iterator &other) const -> bool {
            return (lexer == other.lexer && current_token == other.current_token);
        }
        auto operator!=(const Iterator &other) const -> bool { return !(*this == other); }

      private:
        std::optional<value_type> current_token = std::nullopt;
        Lexer *lexer = nullptr;
    };

    inline auto begin() -> Iterator { return Iterator(this); }
    static inline auto end() -> Iterator { return {}; }

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

auto collect_tokens(const std::string_view source) -> std::pair<std::vector<Token>, std::vector<Error>>;

} // namespace jp
