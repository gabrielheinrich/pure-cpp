cmake_minimum_required (VERSION 3.2)

project (pure-cpp-test-package)

include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup (TARGETS)

add_executable (test test.cpp)
target_link_libraries (test CONAN_PKG::pure-cpp)
set_property (TARGET test PROPERTY CXX_STANDARD 17)
