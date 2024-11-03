add_library(Common STATIC error.cpp parser_helper.cpp)

# target_link_libraries(Common PRIVATE)

target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
