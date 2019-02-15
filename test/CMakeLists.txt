enable_testing()

add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Build and run all the tests and examples.")

add_executable (pure-cpp-test EXCLUDE_FROM_ALL pure-cpp-test.cpp)
set_target_properties (pure-cpp-test PROPERTIES OUTPUT_NAME pure-cpp-test)
target_link_libraries (pure-cpp-test pure-cpp)
target_compile_definitions (pure-cpp-test PUBLIC CATCH_CONFIG_MAIN)
add_test("test/pure-cpp-test" pure-cpp-test)
add_dependencies (check pure-cpp-test)

add_executable (readme-example EXCLUDE_FROM_ALL readme-example.cpp)
set_target_properties (readme-example PROPERTIES OUTPUT_NAME readme-example)
target_link_libraries (readme-example pure-cpp)
add_test ("/test/readme-example" readme-example)
add_dependencies (check readme-example)