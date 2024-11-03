#pragma once

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

constexpr auto is_numeric(char c) -> bool { return c >= '0' && c <= '9'; }
constexpr auto is_lowercase_alphabetic(char c) -> bool { return (c >= 'a' && c <= 'z'); }
constexpr auto is_uppercase_alphabetic(char c) -> bool { return (c >= 'A' && c <= 'Z'); }
constexpr auto is_alphabetic(char c) -> bool { return is_lowercase_alphabetic(c) || is_uppercase_alphabetic(c); }
constexpr auto is_alphanumeric(char c) -> bool { return is_alphabetic(c) || is_numeric(c); }
constexpr auto is_hex_digit(char c) -> bool {
    return is_numeric(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}
