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
using JSONInteger = std::int64_t;
using JSONDouble = double;
struct JSONValue {
    using ValueType = std::variant<JSONNull, bool, JSONInteger, JSONDouble, std::string, JSONObject, JSONArray>;

    ValueType value;

    [[nodiscard]] auto is_null() const -> bool { return std::holds_alternative<JSONNull>(value); }
    [[nodiscard]] auto is_bool() const -> bool { return std::holds_alternative<bool>(value); }
    [[nodiscard]] auto is_double() const -> bool { return std::holds_alternative<JSONDouble>(value); }
    [[nodiscard]] auto is_integer() const -> bool { return std::holds_alternative<JSONInteger>(value); }
    [[nodiscard]] auto is_string() const -> bool { return std::holds_alternative<std::string>(value); }
    [[nodiscard]] auto is_object() const -> bool { return std::holds_alternative<JSONObject>(value); }
    [[nodiscard]] auto is_array() const -> bool { return std::holds_alternative<JSONArray>(value); }
    [[nodiscard]] auto is_numeric() const -> bool { return is_double() || is_integer(); }

    [[nodiscard]] auto as_object() const -> const JSONObject & { return std::get<JSONObject>(value); }
    [[nodiscard]] auto as_array() const -> const JSONArray & { return std::get<JSONArray>(value); }
    [[nodiscard]] auto as_string() const -> const std::string & { return std::get<std::string>(value); }
    [[nodiscard]] auto as_double() const -> JSONDouble { return std::get<JSONDouble>(value); }
    [[nodiscard]] auto as_integer() const -> JSONInteger { return std::get<JSONInteger>(value); }
    [[nodiscard]] auto as_bool() const -> bool { return std::get<bool>(value); }

    // Make sure to check whether the type is numeric before calling this
    [[nodiscard]] auto to_double() const -> JSONDouble {
        if (is_double()) {
            return as_double();
        }

        return static_cast<JSONDouble>(as_integer());
    }

    [[nodiscard]] auto type_id() const -> std::size_t { return value.index(); }
    [[nodiscard]] auto type_str() const -> std::string {
        return std::visit(overloaded{[](const JSONNull &) -> std::string { return "null"; },
                                     [](bool) -> std::string { return "bool"; },
                                     [](JSONDouble) -> std::string { return "double"; },
                                     [](JSONInteger) -> std::string { return "integer"; },
                                     [](const std::string &) -> std::string { return "string"; },
                                     [](const JSONObject &) -> std::string { return "object"; },
                                     [](const JSONArray &) -> std::string { return "array"; }},
                          value);
    }

    JSONValue() = default;
    explicit JSONValue(JSONNull /*null*/) : value(JSONNull{}) {}
    explicit JSONValue(bool b) : value(b) {}
    explicit JSONValue(JSONDouble d) : value(d) {}
    explicit JSONValue(JSONInteger d) : value(d) {}
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
    if (arr.empty()) {
        return "[]";
    }

    auto str = std::string{"[ "};

    for (const auto &value : arr) {
        str += to_string(value);
        str += ", ";
    }

    if (!str.empty()) {
        str.pop_back();
        str.pop_back();
    }

    return str + " ]";
}

inline auto to_string(const JSONObject &obj) -> std::string {
    if (obj.empty()) {
        return "{}";
    }

    auto str = std::string{"{ "};

    for (const auto &[key, value] : obj) {
        str += '"';
        str += key;
        str += '"';
        str += ": ";
        str += to_string(value);
        str += ", ";
    }

    if (!str.empty() && str.size() > 1) {
        str.pop_back();
        str.pop_back();
    }

    return str + " }";
}

inline auto to_string(const JSONValue &val) -> std::string {
    return std::visit(overloaded{[](const JSONNull &) -> std::string { return "null"; },
                                 [](bool b) -> std::string { return b ? "true" : "false"; },
                                 [](JSONDouble d) -> std::string { return std::to_string(d); },
                                 [](JSONInteger i) -> std::string { return std::to_string(i); },
                                 [](const std::string &s) -> std::string { return '"' + s + '"'; },
                                 [](const JSONObject &obj) -> std::string { return to_string(obj); },
                                 [](const JSONArray &arr) -> std::string { return to_string(arr); }},
                      val.value);
    return {};
}

} // namespace jp
