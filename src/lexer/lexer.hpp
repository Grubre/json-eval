#pragma once

#include "error.hpp"
#include "expected.hpp"
#include "token.hpp"
#include <functional>
#include <memory>
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

    class LexerIterator {
      public:
        using iterator_category = std::input_iterator_tag;
        using value_type = jp::expected<Token, Error>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type *;
        using reference = const value_type &;

        // Constructors
        explicit LexerIterator(Lexer *lexer) : lexer(lexer), current_token(std::make_unique<value_type>(advance())) {}
        LexerIterator() = default;

        // Dereference
        auto operator*() const -> reference { return *current_token; }
        auto operator->() const -> pointer { return current_token.get(); }

        // Pre-increment
        auto operator++() -> LexerIterator & {
            current_token = std::make_unique<value_type>(advance());
            return *this;
        }

        // Post-increment
        auto operator++(int) -> LexerIterator {
            LexerIterator temp = std::move(*this);
            ++(*this);
            return temp;
        }

        // Equality and inequality
        auto operator==(const LexerIterator &other) const -> bool {
            return (is_end() && other.is_end()) ||
                   (current_token && other.current_token && *current_token == *other.current_token);
        }

        auto operator!=(const LexerIterator &other) const -> bool { return !(*this == other); }

      private:
        Lexer *lexer = nullptr;
        std::unique_ptr<value_type> current_token;

        // Advance the iterator to the next token
        auto advance() -> value_type {
            if (lexer != nullptr) {
                if (auto token = lexer->next_token()) {
                    return std::move(*token);
                }
            }
            return Error{.source = "Lexer",
                         .message = "End of input reached",
                         .line = lexer->line_number,
                         .column = lexer->column_number};
        }

        [[nodiscard]] auto is_end() const -> bool { return !current_token || current_token->has_error(); }
    };

    // Begin and end functions for Lexer
    static inline auto begin(Lexer &lexer) -> LexerIterator { return LexerIterator(&lexer); }

    static inline auto end(Lexer & /*lexer*/) -> LexerIterator { return {}; }

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
