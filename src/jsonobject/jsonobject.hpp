#pragma once

#include "common.hpp"
#include <vector>
#include <variant>
#include <unordered_map>
#include <string>
#include <string_view>

namespace jp {

struct JSONNull {};
using JSONObject = std::unordered_map<std::string, struct JSONValue>;
using JSONArray = std::vector<struct JSONValue>;
using JSONNumber = double;
struct JSONValue {
    using ValueType = std::variant<JSONNull, bool, JSONNumber, std::string, JSONObject, JSONArray>;

    ValueType value;

    [[nodiscard]] auto is_null() const -> bool { return std::holds_alternative<JSONNull>(value); }
    [[nodiscard]] auto is_bool() const -> bool { return std::holds_alternative<bool>(value); }
    [[nodiscard]] auto is_number() const -> bool { return std::holds_alternative<JSONNumber>(value); }
    [[nodiscard]] auto is_string() const -> bool { return std::holds_alternative<std::string>(value); }
    [[nodiscard]] auto is_object() const -> bool { return std::holds_alternative<JSONObject>(value); }
    [[nodiscard]] auto is_array() const -> bool { return std::holds_alternative<JSONArray>(value); }

    JSONValue() = default;
    explicit JSONValue(JSONNull /*null*/) : value(JSONNull{}) {}
    explicit JSONValue(bool b) : value(b) {}
    explicit JSONValue(double d) : value(d) {}
    explicit JSONValue(const std::string &s) : value(s) {}
    explicit JSONValue(const char *s) : value(std::string(s)) {}
    explicit JSONValue(const JSONObject &obj) : value(obj) {}
    explicit JSONValue(const JSONArray &arr) : value(arr) {}
};

inline auto keys(const JSONObject &obj) -> std::vector<std::string_view> {
    auto keys = std::vector<std::string_view>{};

    for (const auto &[key, _] : obj) {
        keys.push_back(key);
    }

    return keys;
}

inline auto to_string(const JSONNull & /*unused*/) -> std::string { return "null"; }

inline auto to_string(const JSONValue &val) -> std::string;

inline auto to_string(const JSONArray &arr) -> std::string {
    auto str = std::string{'['};

    for (const auto &value : arr) {
        str += to_string(value);
        str += ", ";
    }

    if (!str.empty()) {
        str.pop_back();
        str.pop_back();
    }

    return str + ']';
}

inline auto to_string(const JSONObject &obj) -> std::string {
    auto str = std::string{'{'};

    for (const auto &[key, value] : obj) {
        str += key;
        str += ": ";
        str += to_string(value);
        str += ", ";
    }

    if (!str.empty() && str.size() > 1) {
        str.pop_back();
        str.pop_back();
    }

    return str + '}';
}

inline auto to_string(const JSONValue &val) -> std::string {
    return std::visit(overloaded{[](const JSONNull &) -> std::string { return "null"; },
                                 [](bool b) -> std::string { return b ? "true" : "false"; },
                                 [](double d) -> std::string { return std::to_string(d); },
                                 [](const std::string &s) -> std::string { return '"' + s + '"'; },
                                 [](const JSONObject &obj) -> std::string { return to_string(obj); },
                                 [](const JSONArray &arr) -> std::string { return to_string(arr); }},
                      val.value);
    return {};
}

} // namespace jp
