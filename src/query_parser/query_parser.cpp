#include "query_parser.hpp"
#include "query_token.hpp"
#include <iostream>

namespace query {

auto Parser::chop() -> std::optional<query::Token> {
    if (tokens.empty()) {
        return std::nullopt;
    }

    auto token = tokens.front();
    tokens = tokens.subspan(1);
    return token;
}

[[nodiscard]] auto Parser::peek() const -> const query::Token * {
    if (tokens.empty()) {
        return nullptr;
    }

    return &tokens.front();
}

void Parser::push_err(Error &&err) { errors.push_back(std::move(err)); }

void Parser::push_err(std::string &&message, unsigned column) {
    errors.push_back(Error{.message = std::move(message), .line = 1, .column = column});
}

void Parser::throw_unexpected_token(std::string &&expected, const Token &unexpected) {
    push_err(std::format("Unexpected token: Expected {}, instead found {}", expected, to_string(unexpected.token_type)),
             unexpected.col);
}

void Parser::throw_unexpected_end_of_stream(std::string &&expected) {
    push_err(std::format("Unexpected end of stream: Expected {}", expected), 0);
}

auto Parser::get_errors() -> std::span<Error> { return errors; }

auto Parser::parse() -> std::optional<query::Expression> {
    if (tokens.empty()) {
        return std::nullopt;
    }

    return parse_expression();
}

auto Parser::parse_value() -> std::optional<query::Value> {
    auto maybe_token = chop();

    if (!maybe_token) {
        return std::nullopt;
    }

    auto token = *maybe_token;

    return std::visit(
        overloaded{[&](const query::Double &d) -> std::optional<query::Value> { return query::Double{d.value}; },
                   [&](const query::Integer &i) -> std::optional<query::Value> { return query::Integer{i.value}; },
                   [&](const auto &unexpected) -> std::optional<query::Value> {
                       throw_unexpected_token("Value", token);
                       return std::nullopt;
                   }},
        token.token_type);
}

auto Parser::parse_expression() -> std::optional<query::Expression> {
    auto lhs = parse_value();
    if (!lhs.has_value()) {
        return std::nullopt;
    }

    if (tokens.empty()) {
        return lhs;
    }

    /*auto op = *chop();*/
    /*if (!std::holds_alternative<query::Operator>(op.token_type)) {*/
    /*    throw_unexpected_token("Operator", op);*/
    /*    return std::nullopt;*/
    /*}*/
    /**/
    /*auto rhs = parse_value();*/
    /*if (!rhs.has_value()) {*/
    /*    return std::nullopt;*/
    /*}*/

    std::cerr << "not yet implemented" << std::endl;
    assert(false);
    /*return query::Expression{.lhs = std::move(lhs.value()), .op = std::get<query::Operator>(op.token_type), .rhs =
     * std::move(rhs.value())};*/
}

} // namespace query
