#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "lexer.hpp"
#include "expected.hpp"
#include "token.hpp"
#include "error.hpp"
#include <fstream>
#include "test_shared.hpp"

using namespace jp;

TEST_CASE("JSONTestSuite") {
    auto dir = std::filesystem::path(TESTS_DIR);

    auto [filename, filecontent] = read_test_files(dir);
    auto lexer = Lexer(filecontent);
    INFO("Filename: " << filename);

    CHECK(true);
}
