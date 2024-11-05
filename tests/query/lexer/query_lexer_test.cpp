#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "query_lexer.hpp"

using namespace query;

TEST_SUITE("Query Lexer") {
    TEST_CASE("Lexer recognizes simple JSON paths") {
        const auto source = R"(a.b.c.ce.x)";

        auto lexer = query::Lexer{source};

        while (auto token = lexer.next_token()) {
            CHECK(token->has_value());
        }
    }

    TEST_CASE("Lexer rejects unknown characters") {
        const auto source = R"(a.b.c.ce.x$)";

        auto lexer = query::Lexer{source};

        bool has_error = false;
        while (auto token = lexer.next_token()) {
            has_error = has_error || token->has_error();
        }

        CHECK(has_error);
    }

    TEST_CASE("Lexer recognizes simple JSON paths with subscripts") {
        const auto source = R"(a.b[0].c.ce.x)";

        auto lexer = query::Lexer{source};

        while (auto token = lexer.next_token()) {
            CHECK(token->has_value());
        }
    }

    TEST_CASE("Lexer recognizes simple JSON paths with subscripts and negative numbers") {
        const auto source = R"(a.b[-1].c.ce.x)";

        auto lexer = query::Lexer{source};

        while (auto token = lexer.next_token()) {
            CHECK(token->has_value());
        }
    }

    TEST_CASE("Lexer recognizes binary operations") {
        const auto source = R"(1 + 2 - 3 * 4 / 5)";

        auto lexer = query::Lexer{source};

        while (auto token = lexer.next_token()) {
            CHECK(token->has_value());
        }
    }

    TEST_CASE("Lexer recognizes unary operations") {
        const auto source = R"(-1 + -2 - -3 * -4 / -5)";

        auto lexer = query::Lexer{source};

        while (auto token = lexer.next_token()) {
            CHECK(token->has_value());
        }
    }
}
