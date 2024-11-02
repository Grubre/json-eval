#include "parser.hpp"
#include "common.hpp"
#include "token.hpp"
#include <iostream>

namespace jp {
auto Parser::chop() -> std::optional<Token> {
    if (tokens.empty()) {
        return std::nullopt;
    }

    auto token = tokens.front();
    tokens = tokens.subspan(1);

    return token;
}

[[nodiscard]] auto Parser::peek() const -> const jp::Token * {
    if (tokens.empty()) {
        return nullptr;
    }

    return &tokens.front();
}

auto Parser::parse_object() -> std::optional<JSONObject> {
    chop(); // consume LBrace
    JSONObject obj;

    if (tokens.empty()) {
        return std::nullopt;
    }

    // the object is empty
    if (std::holds_alternative<jp::RBrace>(peek()->token_type)) {
        chop(); // consume RBrace
        return obj;
    }

    while (!tokens.empty()) {
        auto key = *chop();

        if (!std::holds_alternative<jp::String>(key.token_type)) {
            // throw unexpected token error
            return std::nullopt;
        }

        auto colon = chop();
        if (!colon || !std::holds_alternative<jp::Colon>(colon->token_type)) {
            // throw unexpected token error
            return std::nullopt;
        }

        auto value = parse_value();

        if (!value) {
            // throw unexpected token error
            return std::nullopt;
        }

        obj[std::get<jp::String>(key.token_type).value] = *value;

        auto delimiter = chop();

        if (!delimiter) {
            // throw unexpected token error
            return std::nullopt;
        }

        if (std::holds_alternative<jp::RBrace>(delimiter->token_type)) {
            return obj;
        }

        if (!std::holds_alternative<jp::Comma>(delimiter->token_type)) {
            // throw unexpected token error
            return std::nullopt;
        }

        chop(); // consume comma
    }

    return std::nullopt;
}

auto Parser::parse_value() -> std::optional<JSONValue> {
    if (tokens.empty()) {
        return std::nullopt;
    }

    auto token = *chop();

    std::visit(overloaded{[&](jp::LBrace) -> std::optional<JSONValue> {
                              auto obj = parse_object();
                              if (obj) {
                                  return JSONValue(*obj);
                              }
                              return std::nullopt;
                          },
                          [&](jp::LBracket) -> std::optional<JSONValue> {
                              exit(20);
                              /*auto arr = parse_array();*/
                              /*if (arr) {*/
                              /*    return JSONValue(*arr);*/
                              /*}*/
                              return std::nullopt;
                          },
                          [&](jp::String &&s) -> std::optional<JSONValue> { return JSONValue(s.value); },
                          [&](jp::Number &&n) -> std::optional<JSONValue> { return JSONValue(n.value); },
                          [&](jp::True) -> std::optional<JSONValue> { return JSONValue(true); },
                          [&](jp::False) -> std::optional<JSONValue> { return JSONValue(false); },
                          [&](jp::Null) -> std::optional<JSONValue> { return JSONValue(JSONNull{}); },
                          [&](auto) -> std::optional<JSONValue> { return std::nullopt; }},
               token.token_type);

    return std::nullopt;
}

auto Parser::parse() -> std::optional<JSONValue> {
    if (tokens.empty()) {
        return std::nullopt;
    }

    auto token = *chop();

    std::visit(overloaded{[&](jp::LBrace) -> std::optional<JSONValue> {
                              auto obj = parse_object();
                              if (obj) {
                                  return JSONValue(*obj);
                              }
                              return std::nullopt;
                          },
                          [&](auto) -> std::optional<JSONValue> { return std::nullopt; }},
               token.token_type);

    return std::nullopt;
}
} // namespace jp
