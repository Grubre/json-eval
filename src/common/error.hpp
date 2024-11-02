#pragma once

#include <string>

struct Error {
    std::string source;
    std::string message;
    unsigned line;
    unsigned column;

    bool is_warning = false;

    auto operator==(const Error &other) const -> bool;
};

void display_error(const Error &error);

template <typename T>
concept ThrowsError = requires(T t) { t.get_errors(); };
