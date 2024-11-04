#pragma once
#include <string_view>
#include "expected.hpp"
#include "error.hpp"
#include "token.hpp"
#include <functional>

constexpr auto default_ending_predicate = [](char c) -> bool { return c == '"'; };

// Expects a string_view that starts with the first character after the opening '"'
auto parse_str(std::string_view &source, const std::function<bool(char)> &ending_predicate = default_ending_predicate)
    -> jp::expected<std::string, Error>;
auto parse_num(std::string_view &source) -> jp::expected<jp::Number, Error>;
