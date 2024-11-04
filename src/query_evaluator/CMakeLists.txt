add_library(QueryEvaluator STATIC query_evaluator.cpp)

target_link_libraries(QueryEvaluator PRIVATE Common JSONObject QueryParser)

target_include_directories(QueryEvaluator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
