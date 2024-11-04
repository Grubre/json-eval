#include "parser.hpp"
#include "common.hpp"
#include "token.hpp"
#include "lexer.hpp"
#include <format>

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

void Parser::push_err(Error &&err) { errors.push_back(std::move(err)); }

void Parser::push_err(std::string &&message, unsigned line, unsigned column) {
    errors.push_back(Error{.message = std::move(message), .line = line, .column = column});
}

void Parser::throw_unexpected_token(std::string &&expected, const Token &unexpected) {
    push_err(std::format("Unexpected token: Expected {}, instead found {}", expected, to_string(unexpected.token_type)),
             unexpected.row, unexpected.col);
}

void Parser::throw_unexpected_end_of_stream(std::string &&expected) {
    push_err(std::format("Unexpected end of stream: Expected {}", expected), 0, 0);
}

auto Parser::get_errors() -> std::span<Error> { return errors; }

auto Parser::parse_object() -> std::optional<JSONObject> {
    auto obj = JSONObject{};

    if (tokens.empty()) {
        return std::nullopt;
    }

    // the object is empty
    if (std::holds_alternative<jp::RBrace>(peek()->token_type)) {
        chop(); // consume RBrace
        return obj;
    }

    while (!tokens.empty()) {
        auto maybe_key = *chop();

        if (!std::holds_alternative<jp::String>(maybe_key.token_type)) {
            throw_unexpected_token("a key (String)", maybe_key);
            return std::nullopt;
        }

        auto maybe_colon = chop();
        if (!maybe_colon || !std::holds_alternative<jp::Colon>(maybe_colon->token_type)) {
            throw_unexpected_token(":", *maybe_colon);
            return std::nullopt;
        }

        auto maybe_value = parse_value();

        if (!maybe_value) {
            return std::nullopt;
        }

        obj[std::get<jp::String>(maybe_key.token_type).value] = *maybe_value;

        auto delimiter = chop();

        if (!delimiter) {
            throw_unexpected_end_of_stream("Expected ',' or '}'");
            return std::nullopt;
        }

        if (std::holds_alternative<jp::RBrace>(delimiter->token_type)) {
            return obj;
        }

        if (!std::holds_alternative<jp::Comma>(delimiter->token_type)) {
            throw_unexpected_token("','", *delimiter);
            return std::nullopt;
        }
    }

    return std::nullopt;
}

auto Parser::parse_array() -> std::optional<JSONArray> {
    auto arr = JSONArray{};

    if (tokens.empty()) {
        return std::nullopt;
    }

    // the array is empty
    if (std::holds_alternative<jp::RBracket>(peek()->token_type)) {
        chop(); // consume RBracket
        return arr;
    }

    while (!tokens.empty()) {
        auto value = parse_value();

        if (!value) {
            return std::nullopt;
        }

        arr.push_back(*value);

        auto delimiter = chop();

        if (!delimiter) {
            throw_unexpected_end_of_stream("Expected ',' or ']'");
            return std::nullopt;
        }

        if (std::holds_alternative<jp::RBracket>(delimiter->token_type)) {
            return arr;
        }

        if (!std::holds_alternative<jp::Comma>(delimiter->token_type)) {
            throw_unexpected_token("','", *delimiter);
            return std::nullopt;
        }
    }

    return std::nullopt;
}

auto Parser::parse_value() -> std::optional<JSONValue> {
    if (tokens.empty()) {
        return std::nullopt;
    }

    auto token = *chop();

    return std::visit(
        overloaded{[&](jp::LBrace) -> std::optional<JSONValue> {
                       auto obj = parse_object();
                       if (obj) {
                           return JSONValue(*obj);
                       }
                       return std::nullopt;
                   },
                   [&](jp::LBracket) -> std::optional<JSONValue> {
                       auto arr = parse_array();
                       if (arr) {
                           return JSONValue(*arr);
                       }
                       return std::nullopt;
                   },
                   [&](const jp::String &s) -> std::optional<JSONValue> { return JSONValue(s.value); },
                   [&](const jp::Number &n) -> std::optional<JSONValue> {
                       return std::visit(
                           overloaded{[&](int64_t i) -> std::optional<JSONValue> { return JSONValue(JSONInteger(i)); },
                                      [&](double d) -> std::optional<JSONValue> { return JSONValue(JSONDouble(d)); }},
                           n.value);
                   },
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

    return parse_value();
}

auto parse(const std::string_view &json) -> expected<JSONValue, std::vector<Error>> {
    auto [tokens, errors] = jp::collect_tokens(json);

    if (!errors.empty()) {
        return errors;
    }

    auto parser = Parser(tokens);

    auto result = parser.parse();

    if (result) {
        return *result;
    }

    for (auto &error : parser.get_errors()) {
        errors.push_back(error);
    }

    return errors;
}
} // namespace jp
