set(EXEC_NAME "json-eval")

set(SOURCE_FILES
    main.cpp
)

# Used for visual studio source grouping
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_executable(${EXEC_NAME} ${SOURCE_FILES})

set(MAIN_FLAGS ${COMPILE_FLAGS})

if(SANITIZER_AVAILABLE_AND_SET)
  set(MAIN_FLAGS ${MAIN_FLAGS} ${SANITIZER_FLAGS})
  target_link_libraries(${EXEC_NAME} ${SANITIZER_FLAGS})
endif()

target_compile_options(${EXEC_NAME} PRIVATE ${MAIN_FLAGS})
target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
