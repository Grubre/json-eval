#pragma once

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

} // namespace jp
