#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "lexer.hpp"
#include "expected.hpp"
#include "token.hpp"
#include "error.hpp"

using namespace jp;

TEST_SUITE("Lexer") {

    TEST_CASE("Lexer recognizes keywords") {
        Lexer lexer("true false null");

        auto token1 = lexer.next_token();
        REQUIRE(token1.has_value());
        REQUIRE(token1->has_value());
        CHECK(std::holds_alternative<True>(token1->value().token_type));

        auto token2 = lexer.next_token();
        REQUIRE(token2.has_value());
        REQUIRE(token2->has_value());
        CHECK(std::holds_alternative<False>(token2->value().token_type));

        auto token3 = lexer.next_token();
        REQUIRE(token3.has_value());
        REQUIRE(token3->has_value());
        CHECK(std::holds_alternative<Null>(token3->value().token_type));
    }

    TEST_CASE("Lexer rejects invalid keywords") {
        Lexer lexer("trues");

        auto token = lexer.next_token();
        REQUIRE(token.has_value());
        REQUIRE(token->has_error());
    }

    TEST_CASE("Lexer recognizes numbers") {
        Lexer lexer("123 456.78");

        // First token: integer number
        auto token1 = lexer.next_token();
        REQUIRE(token1.has_value());
        REQUIRE(token1->has_value());
        CHECK(token1->value().row == 1);
        CHECK(token1->value().col == 1);
        REQUIRE(std::holds_alternative<Number>(token1->value().token_type));
        CHECK(std::get<Number>(token1->value().token_type).value == 123.);

        // Second token: floating point number
        auto token2 = lexer.next_token();
        REQUIRE(token2.has_value());
        REQUIRE(token2->has_value());
        CHECK(token2->value().row == 1);
        CHECK(token2->value().col == 5);
        REQUIRE(std::holds_alternative<Number>(token2->value().token_type));
        CHECK(std::get<Number>(token2->value().token_type).value == 456.78);
    }

    TEST_CASE("Lexer recognizes scientific notation") {
        Lexer lexer("1e3 1.2e-3");

        // First token: 1e3
        auto token1 = lexer.next_token();
        REQUIRE(token1.has_value());
        REQUIRE(token1->has_value());
        CHECK(token1->value().row == 1);
        CHECK(token1->value().col == 1);
        REQUIRE(std::holds_alternative<Number>(token1->value().token_type));
        CHECK(std::get<Number>(token1->value().token_type).value == 1e3);

        // Second token: 1.2e-3
        auto token2 = lexer.next_token();
        REQUIRE(token2.has_value());
        REQUIRE(token2->has_value());
        CHECK(token2->value().row == 1);
        CHECK(token2->value().col == 5);
        REQUIRE(std::holds_alternative<Number>(token2->value().token_type));
        CHECK(std::get<Number>(token2->value().token_type).value == 1.2e-3);
    }

    TEST_CASE("Lexer rejects invalid scientific notation") {
        Lexer lexer("1e");

        auto token = lexer.next_token();
        REQUIRE(token.has_value());
        REQUIRE(token->has_error());
    }

    TEST_CASE("Lexer recognizes strings") {
        Lexer lexer(R"("hello" "world")");

        // First token: "hello"
        auto token1 = lexer.next_token();
        REQUIRE(token1.has_value());
        REQUIRE(token1->has_value());
        CHECK(token1->value().row == 1);
        CHECK(token1->value().col == 1);
        REQUIRE(std::holds_alternative<String>(token1->value().token_type));
        CHECK(std::get<String>(token1->value().token_type).value == "hello");

        // Second token: "world"
        auto token2 = lexer.next_token();
        REQUIRE(token2.has_value());
        REQUIRE(token2->has_value());
        CHECK(token2->value().row == 1);
        CHECK(token2->value().col == 9);
        REQUIRE(std::holds_alternative<String>(token2->value().token_type));
        CHECK(std::get<String>(token2->value().token_type).value == "world");
    }

    TEST_CASE("Lexer recognizes empty string") {
        Lexer lexer("\"\"");

        auto token = lexer.next_token();
        REQUIRE(token.has_value());
        REQUIRE(token->has_value());
        REQUIRE(std::holds_alternative<String>(token->value().token_type));
        CHECK(std::get<String>(token->value().token_type).value.empty());
    }

    TEST_CASE("Lexer recognizes strings with escaped characters") {
        Lexer lexer(R"("hello\nworld")");

        auto token = lexer.next_token();
        REQUIRE(token.has_value());
        REQUIRE(token->has_value());
        REQUIRE(std::holds_alternative<String>(token->value().token_type));
        CHECK(std::get<String>(token->value().token_type).value == "hello\nworld");
    }

    TEST_CASE("Lexer recognizes invalid strings") {
        Lexer lexer(R"("hello)");

        auto token = lexer.next_token();
        REQUIRE(token.has_value());
        REQUIRE(token->has_error());
    }

    TEST_CASE("LexerIterator basic functionality") {}
}

TEST_SUITE("Lexer::Iterator") {
    TEST_CASE("Empty source input") {
        Lexer lexer("");
        auto it = lexer.begin();
        auto end = lexer.end();

        REQUIRE(it == end);
    }

    TEST_CASE("Valid source input") {
        const auto *const source = R"({["key": "value", "key2": 123.45, "key3": true, "key4": null]})";

        for (const auto &token : Lexer(source)) {
            CHECK(token.has_value());
        }
    }

    TEST_CASE("Invalid source input") {
        const auto *const source = R"(abc e123.123e.e ; ''' ??--)";

        for (const auto &token : Lexer(source)) {
            CHECK(token.has_error());
        }
    }
}
