#pragma once

#include <cassert>
#include <cstdint>
#include <format>
#include <string>
#include <variant>

// JSON tokens
struct LBracket {}; // [
struct RBracket {}; // ]
struct Comma {};    // ,
struct Colon {};    // :
struct LBrace {};   // {
struct RBrace {};   // }
struct Quote {};    // "

struct True {};  // true
struct False {}; // false
struct Null {};  // null

struct Number {
    // FIXME: Might need to be a variant<double, int64_t> to support both int and double
    double value;
};

struct String {
    std::string value;
};

using TokenType =
    std::variant<LBracket, RBracket, Comma, Colon, LBrace, RBrace, Quote, True, False, Null, Number, String>;

struct Token {
    TokenType token_type;
    std::uint32_t row;
    std::uint32_t col;

    // FIXME: Comparing just the indexes and not the actual values of the variant
    //        might not be fully correct.
    auto operator==(const Token &other) const -> bool {
        return token_type.index() == other.token_type.index() && row == other.row && col == other.col;
    }
};

constexpr auto to_string(TokenType token_type) -> std::string {
    return std::visit(
        [](const auto &token) -> std::string {
            using T = std::decay_t<decltype(token)>;

            if constexpr (std::is_same_v<T, LBracket>) {
                return "[";
            } else if constexpr (std::is_same_v<T, RBracket>) {
                return "]";
            } else if constexpr (std::is_same_v<T, Comma>) {
                return ",";
            } else if constexpr (std::is_same_v<T, Colon>) {
                return ":";
            } else if constexpr (std::is_same_v<T, LBrace>) {
                return "{";
            } else if constexpr (std::is_same_v<T, RBrace>) {
                return "}";
            } else if constexpr (std::is_same_v<T, Quote>) {
                return "\"";
            } else if constexpr (std::is_same_v<T, True>) {
                return "true";
            } else if constexpr (std::is_same_v<T, False>) {
                return "false";
            } else if constexpr (std::is_same_v<T, Null>) {
                return "null";
            } else if constexpr (std::is_same_v<T, Number>) {
                return std::to_string(token.value);
            } else if constexpr (std::is_same_v<T, String>) {
                return token.value;
            } else {
                return "";
            }
        },
        token_type);
}
