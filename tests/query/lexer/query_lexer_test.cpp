#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "query_lexer.hpp"

using namespace query;

TEST_SUITE("Query Lexer") {
    TEST_CASE("Query Lexer recognizes simple JSON paths") {
        const auto source = R"(a.b.c.ce.x)";

        auto lexer = query::Lexer{source};

        while (auto token = lexer.next_token()) {
            CHECK(token->has_value());
        }
    }
}
