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
        CHECK(token1.has_value());
        CHECK(token1->has_value());
        CHECK(std::holds_alternative<True>(token1->value().token_type));

        auto token2 = lexer.next_token();
        CHECK(token2.has_value());
        CHECK(token2->has_value());
        CHECK(std::holds_alternative<False>(token2->value().token_type));

        auto token3 = lexer.next_token();
        CHECK(token3.has_value());
        CHECK(token3->has_value());
        CHECK(std::holds_alternative<Null>(token3->value().token_type));
    }

    TEST_CASE("Lexer rejects invalid keywords") {
        Lexer lexer("trues");

        auto token = lexer.next_token();
        CHECK(token.has_value());
        CHECK(token->has_error());
        CHECK(token->error().message == "Unexpected keyword 'trues'");
    }

    TEST_CASE("Lexer recognizes numbers") {
        Lexer lexer("123 456.78");

        // First token: integer number
        auto token1 = lexer.next_token();
        CHECK(token1.has_value());
        CHECK(token1->has_value());
        CHECK(std::holds_alternative<Number>(token1->value().token_type));
        CHECK(token1->value().row == 1);
        CHECK(token1->value().col == 1);
        CHECK(std::get<Number>(token1->value().token_type).value == 123.);

        // Second token: floating point number
        auto token2 = lexer.next_token();
        CHECK(token2.has_value());
        CHECK(token2->has_value());
        CHECK(std::holds_alternative<Number>(token2->value().token_type));
        CHECK(token2->value().row == 1);
        CHECK(token2->value().col == 5);
        CHECK(std::get<Number>(token2->value().token_type).value == 456.78);
    }

    TEST_CASE("Lexer recognizes strings") {
        Lexer lexer(R"("hello" "world")");

        // First token: "hello"
        auto token1 = lexer.next_token();
        CHECK(token1.has_value());
        CHECK(token1->has_value());
        CHECK(std::holds_alternative<String>(token1->value().token_type));
        CHECK(token1->value().row == 1);
        CHECK(token1->value().col == 1);
        CHECK(std::get<String>(token1->value().token_type).value == "hello");

        // Second token: "world"
        auto token2 = lexer.next_token();
        CHECK(token2.has_value());
        CHECK(token2->has_value());
        CHECK(std::holds_alternative<String>(token2->value().token_type));
        CHECK(token2->value().row == 1);
        CHECK(token2->value().col == 9);
        CHECK(std::get<String>(token2->value().token_type).value == "world");
    }

    TEST_CASE("Lexer recognizes empty string") {
        Lexer lexer("\"\"");

        auto token = lexer.next_token();
        CHECK(token.has_value());
        CHECK(token->has_value());
        CHECK(std::holds_alternative<String>(token->value().token_type));
        CHECK(std::get<String>(token->value().token_type).value.empty());
    }

    TEST_CASE("Lexer recognizes strings with escaped characters") {
        Lexer lexer(R"("hello\nworld")");

        auto token = lexer.next_token();
        CHECK(token.has_value());
        CHECK(token->has_value());
        CHECK(std::holds_alternative<String>(token->value().token_type));
        CHECK(std::get<String>(token->value().token_type).value == "hello\nworld");
    }

    TEST_CASE("Lexer recognizes invalid strings") {
        Lexer lexer(R"("hello)");

        auto token = lexer.next_token();
        REQUIRE(token.has_value());
        REQUIRE(token->has_error());
    }
}
