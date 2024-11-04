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

    TEST_CASE("Parse a JSON object with multiple key-value pairs") {
        std::string json = R"({"key1": "value1", "key2": "value2", "key3": "value3"})";
        auto result = parse(json);
        CHECK(result.has_value());
        CHECK(result->is_object());
        auto obj = std::get<JSONObject>(result->value);
        CHECK(obj.size() == 3);
        CHECK(obj["key1"].is_string());
        CHECK(std::get<std::string>(obj["key1"].value) == "value1");
        CHECK(obj["key2"].is_string());
        CHECK(std::get<std::string>(obj["key2"].value) == "value2");
        CHECK(obj["key3"].is_string());
        CHECK(std::get<std::string>(obj["key3"].value) == "value3");
    }

    TEST_CASE("Reject invalid JSON objects") {
        std::string json = R"({"key": value})";
        auto result = parse(json);
        CHECK(result.has_error());
    }

    TEST_CASE("Parse an empty JSON array") {
        std::string json = R"([])";
        auto result = parse(json);
        CHECK(result.has_value());
        CHECK(result->is_array());
    }

    TEST_CASE("Parse a JSON array with a single element") {
        std::string json = R"(["value"])";
        auto result = parse(json);
        CHECK(result.has_value());
        CHECK(result->is_array());
        auto arr = std::get<JSONArray>(result->value);
        CHECK(arr.size() == 1);
        CHECK(arr[0].is_string());
        CHECK(std::get<std::string>(arr[0].value) == "value");
    }

    TEST_CASE("Parse a JSON array with multiple elements") {
        std::string json = R"(["value1", "value2", "value3"])";
        auto result = parse(json);
        CHECK(result.has_value());
        CHECK(result->is_array());
        auto arr = std::get<JSONArray>(result->value);
        CHECK(arr.size() == 3);
        CHECK(arr[0].is_string());
        CHECK(std::get<std::string>(arr[0].value) == "value1");
        CHECK(arr[1].is_string());
        CHECK(std::get<std::string>(arr[1].value) == "value2");
        CHECK(arr[2].is_string());
        CHECK(std::get<std::string>(arr[2].value) == "value3");
    }

    TEST_CASE("Reject invalid JSON arrays") {
        std::string json = R"(["value1", "value2", "value3)";
        auto result = parse(json);
        CHECK(result.has_error());
    }

    TEST_CASE("Parse integer") {
        std::string json = R"(123)";
        auto result = parse(json);
        CHECK(result.has_value());
        CHECK(result->is_integer());
        CHECK(std::get<JSONInteger>(result->value) == 123);
    }

    TEST_CASE("Parse double") {
        std::string json = R"(123.45)";
        auto result = parse(json);
        CHECK(result.has_value());
        CHECK(result->is_double());
        CHECK(std::get<JSONDouble>(result->value) == 123.45);
    }
}
