#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "parser.hpp"

using namespace jp;

TEST_SUITE("Parser") {

    TEST_CASE("Parse an empty JSON object") {
        std::string json = R"({})";
        auto result = parse(json);
        CHECK(result.has_value());
        CHECK(result->is_object());
    }

    TEST_CASE("Parse a JSON object with a single key-value pair") {
        std::string json = R"({"key": "value"})";
        auto result = parse(json);
        CHECK(result.has_value());
        CHECK(result->is_object());
        auto obj = std::get<JSONObject>(result->value);
        CHECK(obj.size() == 1);
        CHECK(obj["key"].is_string());
        CHECK(std::get<std::string>(obj["key"].value) == "value");
    }
}
