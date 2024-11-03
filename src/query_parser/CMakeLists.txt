add_library(QueryParser STATIC query_parser.cpp query_lexer.cpp)

target_link_libraries(QueryParser PRIVATE Common JSONObject)

target_include_directories(QueryParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
