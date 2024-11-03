#pragma once
#include <string_view>
#include "expected.hpp"
#include "error.hpp"
#include "token.hpp"

// Expects a string_view that starts with the first character after the opening '"'
auto parse_str(std::string_view &source) -> jp::expected<std::string, Error>;
auto parse_num(std::string_view &source) -> jp::expected<jp::Number, Error>;
