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

    TEST_CASE("LexerIterator basic functionality") {
        using namespace jp;

        SUBCASE("Iterate through JSON tokens") {
            std::string json = R"({"key": "value", "num": 123, "bool": true, "nullVal": null})";
            Lexer lexer(json);
            std::vector<TokenType> expected_tokens = {
                LBrace{}, String{"key"},     Colon{}, String{"value"}, Comma{}, String{"num"},
                Colon{},  Number{123},       Comma{}, String{"bool"},  Colon{}, True{},
                Comma{},  String{"nullVal"}, Colon{}, Null{},          RBrace{}};

            auto it = Lexer::begin(lexer);
            auto end_it = Lexer::end(lexer);

            for (const auto &expected : expected_tokens) {
                REQUIRE(it != end_it);
                CHECK(it->has_value());
                CHECK(it->value().token_type == expected);
                ++it;
            }
            CHECK(it == end_it);
        }

        SUBCASE("Empty JSON input") {
            std::string json = "";
            Lexer lexer(json);
            auto it = Lexer::begin(lexer);
            auto end_it = Lexer::end(lexer);
            CHECK(it == end_it);
        }

        SUBCASE("JSON with only whitespace") {
            std::string json = "    \n\t  ";
            Lexer lexer(json);
            auto it = Lexer::begin(lexer);
            auto end_it = Lexer::end(lexer);
            CHECK(it == end_it);
        }

        SUBCASE("JSON with unexpected token") {
            std::string json = R"({ "valid": true, @)";
            Lexer lexer(json);

            auto it = Lexer::begin(lexer);
            auto end_it = Lexer::end(lexer);

            std::vector<TokenType> expected_tokens = {LBrace{}, String{"valid"}, Colon{}, True{}, Comma{}};

            for (const auto &expected : expected_tokens) {
                REQUIRE(it != end_it);
                CHECK(it->has_value());
                CHECK(it->value().token_type == expected);
                ++it;
            }

            REQUIRE(it != end_it);
            CHECK(it->has_error());
            CHECK(it->error().message == "Unexpected character '@'");
            ++it;
            CHECK(it == end_it);
        }

        SUBCASE("Iterating over numbers and strings") {
            std::string json = R"([123, 456, "text", "another"])";
            Lexer lexer(json);

            auto it = Lexer::begin(lexer);
            auto end_it = Lexer::end(lexer);

            std::vector<TokenType> expected_tokens = {LBracket{},  Number{123},       Comma{},
                                                      Number{456}, Comma{},           String{"text"},
                                                      Comma{},     String{"another"}, RBracket{}};

            for (const auto &expected : expected_tokens) {
                REQUIRE(it != end_it);
                CHECK(it->has_value());
                CHECK(it->value().token_type == expected);
                ++it;
            }
            CHECK(it == end_it);
        }

        SUBCASE("Complex JSON structure") {
            std::string json = R"({"nested": {"array": [1, 2, 3], "bool": false}})";
            Lexer lexer(json);

            auto it = Lexer::begin(lexer);
            auto end_it = Lexer::end(lexer);

            std::vector<TokenType> expected_tokens = {LBrace{}, String{"nested"}, Colon{},    LBrace{}, String{"array"},
                                                      Colon{},  LBracket{},       Number{1},  Comma{},  Number{2},
                                                      Comma{},  Number{3},        RBracket{}, Comma{},  String{"bool"},
                                                      Colon{},  False{},          RBrace{},   RBrace{}};

            for (const auto &expected : expected_tokens) {
                REQUIRE(it != end_it);
                CHECK(it->has_value());
                CHECK(it->value().token_type == expected);
                ++it;
            }
            CHECK(it == end_it);
        }
    }
}
