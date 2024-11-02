#include <iostream>
#include "error.hpp"

auto Error::operator==(const Error &other) const -> bool {
    return (source == other.source && message == other.message && line == other.line && column == other.column &&
            is_warning == other.is_warning);
}

void display_error(const Error &error) {
    std::cout << "Error: " << error.source << ":" << error.line << ":" << error.column << ": " << error.message
              << std::endl;
}
