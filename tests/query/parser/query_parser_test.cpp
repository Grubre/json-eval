#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "query_lexer.hpp"
#include "query_parser.hpp"

using namespace query;

TEST_SUITE("Query Parser") {

    TEST_CASE("Parser accepts paths") {
        SUBCASE("Single path") {
            const std::string query = R"(a)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Path>>(query_parsed.value()));
            CHECK_EQ(std::get<std::unique_ptr<Path>>(query_parsed.value())->id.identifier, "a");
            CHECK(!std::get<std::unique_ptr<Path>>(query_parsed.value())->subscript.has_value());
            ;
            CHECK(!std::get<std::unique_ptr<Path>>(query_parsed.value())->next.has_value());
            ;
        }

        SUBCASE("Path with subscript") {
            const std::string query = R"(a[1])";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Path>>(query_parsed.value()));
            CHECK_EQ(std::get<std::unique_ptr<Path>>(query_parsed.value())->id.identifier, "a");
            CHECK(std::get<std::unique_ptr<Path>>(query_parsed.value())->subscript.has_value());
            CHECK(std::holds_alternative<Integer>(*std::get<std::unique_ptr<Path>>(query_parsed.value())->subscript));
            CHECK_EQ(std::get<Integer>(*std::get<std::unique_ptr<Path>>(query_parsed.value())->subscript).value, 1);
            CHECK(!std::get<std::unique_ptr<Path>>(query_parsed.value())->next.has_value());
        }

        SUBCASE("Path with successor") {
            const std::string query = R"(a.b)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Path>>(query_parsed.value()));
            CHECK_EQ(std::get<std::unique_ptr<Path>>(query_parsed.value())->id.identifier, "a");
            CHECK(!std::get<std::unique_ptr<Path>>(query_parsed.value())->subscript.has_value());
            CHECK(std::get<std::unique_ptr<Path>>(query_parsed.value())->next.has_value());
            CHECK_EQ(std::get<std::unique_ptr<Path>>(query_parsed.value())->next.value()->id.identifier, "b");
            CHECK(!std::get<std::unique_ptr<Path>>(query_parsed.value())->next.value()->subscript.has_value());
            CHECK(!std::get<std::unique_ptr<Path>>(query_parsed.value())->next.value()->next.has_value());
        }

        SUBCASE("Path with subscript and successor") {
            const std::string query = R"(a[1].b)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Path>>(query_parsed.value()));
            CHECK_EQ(std::get<std::unique_ptr<Path>>(query_parsed.value())->id.identifier, "a");
            CHECK(std::get<std::unique_ptr<Path>>(query_parsed.value())->subscript.has_value());
            CHECK(std::holds_alternative<Integer>(*std::get<std::unique_ptr<Path>>(query_parsed.value())->subscript));
            CHECK_EQ(std::get<Integer>(*std::get<std::unique_ptr<Path>>(query_parsed.value())->subscript).value, 1);
            CHECK(std::get<std::unique_ptr<Path>>(query_parsed.value())->next.has_value());
            CHECK_EQ(std::get<std::unique_ptr<Path>>(query_parsed.value())->next.value()->id.identifier, "b");
            CHECK(!std::get<std::unique_ptr<Path>>(query_parsed.value())->next.value()->subscript.has_value());
            CHECK(!std::get<std::unique_ptr<Path>>(query_parsed.value())->next.value()->next.has_value());
        }
    }

    TEST_CASE("Parser accepts unary expressions") {
        SUBCASE("Unary minus") {
            const std::string query = R"(-1.23)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Unary>>(query_parsed.value()));
            CHECK_EQ(std::get<Double>(std::get<std::unique_ptr<Unary>>(query_parsed.value())->value).value,
                     doctest::Approx(1.23));
        }

        SUBCASE("Unary minus with path") {
            const std::string query = R"(-a.b)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Unary>>(query_parsed.value()));
            CHECK_EQ(std::get<std::unique_ptr<Path>>(std::get<std::unique_ptr<Unary>>(query_parsed.value())->value)
                         ->id.identifier,
                     "a");
        }
    }

    TEST_CASE("Parser accepts binary expressions") {
        SUBCASE("Addition") {
            const std::string query = R"(1 + 2)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Binary>>(query_parsed.value()));
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Binary>>(query_parsed.value())->lhs).value, 1);
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Binary>>(query_parsed.value())->rhs).value, 2);
        }

        SUBCASE("Subtraction") {
            const std::string query = R"(1 - 2)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Binary>>(query_parsed.value()));
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Binary>>(query_parsed.value())->lhs).value, 1);
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Binary>>(query_parsed.value())->rhs).value, 2);
        }

        SUBCASE("Multiplication") {
            const std::string query = R"(1 * 2)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Binary>>(query_parsed.value()));
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Binary>>(query_parsed.value())->lhs).value, 1);
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Binary>>(query_parsed.value())->rhs).value, 2);
        }

        SUBCASE("Division") {
            const std::string query = R"(1 / 2)";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Binary>>(query_parsed.value()));
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Binary>>(query_parsed.value())->lhs).value, 1);
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Binary>>(query_parsed.value())->rhs).value, 2);
        }
    }

    TEST_CASE("Parser accepts function calls") {
        SUBCASE("Function with one argument") {
            const std::string query = R"(f(1))";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Function>>(query_parsed.value()));
            CHECK_EQ(std::get<std::unique_ptr<Function>>(query_parsed.value())->name.identifier, "f");
            CHECK_EQ(std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments.size(), 1);
            CHECK(std::holds_alternative<Integer>(
                std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments[0]));
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments[0]).value,
                     1);
        }

        SUBCASE("Function with multiple arguments") {
            const std::string query = R"(func(1, 2, 3))";
            auto [query_tokens, query_errors] = query::collect_tokens(query);
            auto query_parser = query::Parser(query_tokens);

            auto query_parsed = query_parser.parse();
            CHECK(query_parsed.has_value());
            CHECK(std::holds_alternative<std::unique_ptr<Function>>(query_parsed.value()));
            CHECK_EQ(std::get<std::unique_ptr<Function>>(query_parsed.value())->name.identifier, "func");
            CHECK_EQ(std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments.size(), 3);
            CHECK(std::holds_alternative<Integer>(
                std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments[0]));
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments[0]).value,
                     1);
            CHECK(std::holds_alternative<Integer>(
                std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments[1]));
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments[1]).value,
                     2);
            CHECK(std::holds_alternative<Integer>(
                std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments[2]));
            CHECK_EQ(std::get<Integer>(std::get<std::unique_ptr<Function>>(query_parsed.value())->arguments[2]).value,
                     3);
        }
    }
}
