set(JSON_TEST_SUITE "${CMAKE_CURRENT_SOURCE_DIR}/test_parsing")

function(create_test test_name source_file)
  add_executable(${test_name} ${source_file})
  target_link_libraries(${test_name} doctest)
  foreach(lib IN LISTS ARGN)
    target_link_libraries(${test_name} ${lib})
  endforeach()
  target_compile_definitions(${test_name} PRIVATE TESTS_DIR="${JSON_TEST_SUITE}")
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

create_test(lexer_test lexer_tests/lexer_test.cpp Common Lexer)
create_test(JSONTestSuite test_suite.cpp Common Lexer)
