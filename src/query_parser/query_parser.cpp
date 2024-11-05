#include "query_parser.hpp"
#include "query_token.hpp"
#include <iostream>
#include <memory>

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
    errors.push_back(Error{.source = "Query", .message = std::move(message), .line = 1, .column = column});
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

    auto expr = parse_expression();

    if (!tokens.empty()) {
        auto maybe_token = chop();
        if (maybe_token.has_value()) {
            push_err(std::format("Unexpected token: '{}'", to_string(maybe_token->token_type)), maybe_token->col);
        }

        return std::nullopt;
    }

    return expr;
}

auto Parser::parse_path(const query::Identifier &first_id) -> std::optional<std::unique_ptr<query::Path>> {
    auto parse_next = [&](std::int64_t col) -> std::optional<std::unique_ptr<Path>> {
        // Parse the next identifier
        const auto maybe_next_id = chop();
        if (!maybe_next_id) {
            push_err(std::format("Unexpected end of stream, expected identifier after '.'"), col);
        }
        const auto &next_id = *maybe_next_id;
        auto next = parse_path(std::get<query::Identifier>(next_id.token_type));

        if (!next.has_value()) {
            return std::nullopt;
        }

        return std::make_unique<Path>(std::move(**next));
    };

    const auto *const maybe_delimiter = peek();

    if (maybe_delimiter == nullptr) {
        return std::make_unique<Path>(query::Path{first_id, std::nullopt, std::nullopt});
    }

    const auto &delimiter = *maybe_delimiter;

    // Check if the identifier has a subscript
    auto subscript = std::optional<Value>{std::nullopt};
    if (std::holds_alternative<query::LBracket>(delimiter.token_type)) {
        chop(); // Consume the opening bracket

        auto value = parse_value();

        if (!value.has_value()) {
            push_err("Expected value after '['", delimiter.col);
            return std::nullopt;
        }

        subscript = std::move(value);

        auto maybe_closing_bracket = chop();
        if (!maybe_closing_bracket) {
            push_err(std::format("Unexpected end of stream, expected ']' after index"), delimiter.col);
            return std::nullopt;
        }

        const auto &closing_bracket = *maybe_closing_bracket;

        if (!std::holds_alternative<query::RBracket>(closing_bracket.token_type)) {
            push_err(std::format("Unexpected token: Expected ']', instead found '{}'",
                                 to_string(closing_bracket.token_type)),
                     closing_bracket.col);
            return std::nullopt;
        }

        const auto *const maybe_dot = peek();

        if (maybe_dot == nullptr) {
            return std::make_unique<Path>(Path{first_id, std::move(subscript), std::nullopt});
        }

        if (!std::holds_alternative<query::Dot>(maybe_dot->token_type)) {
            return std::make_unique<Path>(Path{first_id, std::move(subscript), std::nullopt});
        }

        chop(); // Consume the dot

        auto next = parse_next(maybe_dot->col);

        return std::make_unique<Path>(Path{first_id, std::move(subscript), std::move(next)});
    }

    if (std::holds_alternative<query::Dot>(delimiter.token_type)) {
        chop(); // Consume the dot

        auto next = parse_next(delimiter.col);

        return std::make_unique<Path>(Path{first_id, std::nullopt, std::move(next)});
    }

    return std::make_unique<Path>(Path{first_id, std::nullopt, std::nullopt});
}

auto Parser::parse_value() -> std::optional<query::Value> {
    auto maybe_token = chop();

    if (!maybe_token) {
        return std::nullopt;
    }

    auto token = *maybe_token;

    // Handle parentheses as the beginning of a grouped expression
    if (std::holds_alternative<query::LParen>(token.token_type)) {
        auto expr = parse_expression();
        if (!expr.has_value()) {
            return std::nullopt;
        }

        auto maybe_rparen = chop();
        if (!maybe_rparen || !std::holds_alternative<query::RParen>(maybe_rparen->token_type)) {
            push_err(std::format("Expected ')' after expression. insted found {}", to_string(maybe_rparen->token_type)),
                     token.col);
            return std::nullopt;
        }

        return expr;
    }

    return std::visit(
        overloaded{[&](const query::Double &d) -> std::optional<query::Value> { return query::Double{d.value}; },
                   [&](const query::Integer &i) -> std::optional<query::Value> { return query::Integer{i.value}; },
                   [&](const query::Identifier &i) -> std::optional<query::Value> {
                       const auto *const maybe_lparen = peek();

                       if (maybe_lparen == nullptr) {
                           return parse_path(i);
                       }

                       if (!std::holds_alternative<query::LParen>(maybe_lparen->token_type)) {
                           return parse_path(i);
                       }

                       return parse_function(i);
                   },
                   [&](const query::Minus & /*minus*/) -> std::optional<query::Value> {
                       auto value = parse_value();
                       if (!value.has_value()) {
                           return std::nullopt;
                       }

                       return std::make_unique<query::Unary>(token, std::move(value.value()));
                   },
                   [&](const auto & /*unexpected*/) -> std::optional<query::Value> {
                       throw_unexpected_token("Value", token);
                       return std::nullopt;
                   }},
        token.token_type);
}

auto Parser::parse_expression() -> std::optional<query::Expression> { return parse_term(); }

auto Parser::parse_function(const query::Identifier &name) -> std::optional<query::Value> {
    auto maybe_lparen = chop();
    if (!maybe_lparen) {
        push_err("Expected '(' after function name", 0);
        return std::nullopt;
    }

    if (!std::holds_alternative<query::LParen>(maybe_lparen->token_type)) {
        push_err(std::format("Unexpected token: Expected '(', instead found {}", to_string(maybe_lparen->token_type)),
                 maybe_lparen->col);
        return std::nullopt;
    }

    auto args = std::vector<query::Value>{};

    while (true) {
        auto arg = parse_value();
        if (!arg.has_value()) {
            return std::nullopt;
        }

        args.push_back(std::move(arg.value()));

        auto maybe_comma = chop();
        if (!maybe_comma) {
            break;
        }

        if (std::holds_alternative<query::RParen>(maybe_comma->token_type)) {
            break;
        }

        if (!std::holds_alternative<query::Comma>(maybe_comma->token_type)) {
            push_err(
                std::format("Unexpected token: Expected ',', instead found {}", to_string(maybe_comma->token_type)),
                maybe_comma->col);
            return std::nullopt;
        }
    }

    return std::make_unique<query::Function>(name, std::move(args));
}

auto Parser::parse_term() -> std::optional<query::Value> {
    auto lhs = parse_factor();
    if (!lhs.has_value()) {
        return std::nullopt;
    }

    while (true) {
        const auto *maybe_op = peek();
        if (maybe_op == nullptr) {
            return lhs;
        }

        if (!std::holds_alternative<query::Plus>(maybe_op->token_type) &&
            !std::holds_alternative<query::Minus>(maybe_op->token_type)) {
            return lhs;
        }

        chop(); // Consume the operator

        auto rhs = parse_factor();
        if (!rhs.has_value()) {
            return std::nullopt;
        }

        lhs = std::make_unique<query::Binary>(*maybe_op, std::move(lhs.value()), std::move(rhs.value()));
    }
}

auto Parser::parse_factor() -> std::optional<query::Value> {
    auto lhs = parse_value();
    if (!lhs.has_value()) {
        return std::nullopt;
    }

    while (true) {
        const auto *maybe_op = peek();
        if (maybe_op == nullptr) {
            return lhs;
        }

        if (!std::holds_alternative<query::Star>(maybe_op->token_type) &&
            !std::holds_alternative<query::Slash>(maybe_op->token_type)) {
            return lhs;
        }

        chop(); // Consume the operator

        auto rhs = parse_factor();
        if (!rhs.has_value()) {
            return std::nullopt;
        }

        lhs = std::make_unique<query::Binary>(*maybe_op, std::move(lhs.value()), std::move(rhs.value()));
    }
}

} // namespace query
