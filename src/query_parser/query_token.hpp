#pragma once

#include <cassert>
#include <cstdint>
#include <format>
#include <string>
#include <variant>

namespace query {

#define DEFINE_TOKEN_TYPE(name, ...)                                                                                   \
    struct name {                                                                                                      \
        __VA_ARGS__                                                                                                    \
        auto operator==(const name &other) const -> bool = default;                                                    \
        auto operator!=(const name &other) const -> bool = default;                                                    \
    };
DEFINE_TOKEN_TYPE(Identifier, std::string identifier;)
DEFINE_TOKEN_TYPE(LBracket)
DEFINE_TOKEN_TYPE(RBracket)
DEFINE_TOKEN_TYPE(LParen)
DEFINE_TOKEN_TYPE(RParen)
DEFINE_TOKEN_TYPE(Comma)
DEFINE_TOKEN_TYPE(Dot)
DEFINE_TOKEN_TYPE(Double, double value;)
DEFINE_TOKEN_TYPE(Integer, std::int64_t value;)

DEFINE_TOKEN_TYPE(Plus)
DEFINE_TOKEN_TYPE(Minus)
DEFINE_TOKEN_TYPE(Star)
DEFINE_TOKEN_TYPE(Slash)

using TokenType =
    std::variant<Identifier, LBracket, RBracket, LParen, RParen, Comma, Dot, Double, Integer, Plus, Minus, Star, Slash>;

struct Token {
    TokenType token_type;
    std::uint32_t col;

    auto operator==(const Token &other) const -> bool {
        return token_type.index() == other.token_type.index() && col == other.col;
    }
};

constexpr auto to_string(TokenType token_type) -> std::string {
    return std::visit(
        [](const auto &token) -> std::string {
            using T = std::decay_t<decltype(token)>;

            if constexpr (std::is_same_v<T, Identifier>) {
                return token.identifier;
            } else if constexpr (std::is_same_v<T, LBracket>) {
                return "[";
            } else if constexpr (std::is_same_v<T, RBracket>) {
                return "]";
            } else if constexpr (std::is_same_v<T, Comma>) {
                return ",";
            } else if constexpr (std::is_same_v<T, Dot>) {
                return ".";
            } else if constexpr (std::is_same_v<T, LParen>) {
                return "(";
            } else if constexpr (std::is_same_v<T, RParen>) {
                return ")";
            } else if constexpr (std::is_same_v<T, Plus>) {
                return "+";
            } else if constexpr (std::is_same_v<T, Minus>) {
                return "-";
            } else if constexpr (std::is_same_v<T, Star>) {
                return "*";
            } else if constexpr (std::is_same_v<T, Slash>) {
                return "/";
            } else if constexpr (std::is_same_v<T, Double>) {
                return std::format("{}", token.value);
            } else if constexpr (std::is_same_v<T, Integer>) {
                return std::format("{}", token.value);
            } else {
                assert(false);
            }
        },
        token_type);
}

} // namespace query
