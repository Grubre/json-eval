#pragma once
#include "doctest/doctest.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>

inline auto read_file(const std::string &filename) -> std::optional<std::string> {
    auto file = std::ifstream(filename);

    if (!file) {
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

#define DOCTEST_VALUE_PARAMETERIZED_DATA(data, data_container)                                                         \
    static size_t _doctest_subcase_idx = 0;                                                                            \
    std::for_each(data_container.begin(), data_container.end(), [&](const auto &in) {                                  \
        DOCTEST_SUBCASE((std::string(#data_container "[") + std::to_string(_doctest_subcase_idx++) + "]").c_str()) {   \
            data = in;                                                                                                 \
        }                                                                                                              \
    });                                                                                                                \
    _doctest_subcase_idx = 0

inline auto read_test_files(const std::filesystem::path &dir) -> std::pair<std::string, std::string> {
    std::string filename;

    auto filenames = std::vector<std::string>();
    if (!std::filesystem::exists(dir)) {
        FAIL("Directory " << std::quoted(dir.string()) << " not found.");
    }
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            filenames.push_back(entry.path().filename().string());
        }
    }
    DOCTEST_VALUE_PARAMETERIZED_DATA(filename, filenames);

    INFO("Opening file " << std::quoted(filename));
    auto filepath = std::string(TESTS_DIR) + "/" + filename;

    auto filecontent = read_file(filepath);

    if (!filecontent) {
        FAIL("File " << std::quoted(filepath) << " not found.");
    }

    return {filename, *filecontent};
}
