#include "error.hpp"

auto Error::operator==(const Error &other) const -> bool {
    return (source == other.source && message == other.message && line == other.line && column == other.column &&
            is_warning == other.is_warning);
}

void display_error(const Error &error) {
    /*fmt::print(fg(fmt::color::white) | fmt::emphasis::bold, "{}:{}:{}: ", error.source, error.line, error.column);*/
    /*if (error.is_warning) {*/
    /*    fmt::print(fg(fmt::color::yellow), "Warning: ");*/
    /*} else {*/
    /*    fmt::print(fg(fmt::color::red), "Fatal Error: ");*/
    /*}*/
    /*fmt::print("{}.\n", error.message);*/
}
