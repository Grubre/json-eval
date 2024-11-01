add_subdirectory(common)
add_subdirectory(lexer)

set(EXEC_NAME "json-eval")

add_executable(${EXEC_NAME} main.cpp)

target_link_libraries(${EXEC_NAME} Common Lexer)

set(MAIN_FLAGS ${COMPILE_FLAGS})

if(SANITIZER_AVAILABLE_AND_SET)
  set(MAIN_FLAGS ${MAIN_FLAGS} ${SANITIZER_FLAGS})
  target_link_libraries(${EXEC_NAME} ${SANITIZER_FLAGS})
endif()

target_compile_options(${EXEC_NAME} PRIVATE ${MAIN_FLAGS})
target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Used for visual studio project creation
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
