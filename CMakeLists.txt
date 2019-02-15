cmake_minimum_required (VERSION 3.0)
project (pure-cpp VERSION 0.0.1)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED on)

add_library (pure-cpp INTERFACE)
target_include_directories (pure-cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/immer/>
    $<INSTALL_INTERFACE:include/pure-cpp/>)

set_target_properties (pure-cpp PROPERTIES
    INTERFACE_COMPILE_FEATURES cxx_std_17)

install (TARGETS pure-cpp EXPORT PureCppConfig)
install (EXPORT PureCppConfig DESTINATION lib/cmake/PureCpp)
install (DIRECTORY include/pure DESTINATION include/pure-cpp)
install (DIRECTORY include/immer/immer DESTINATION include/pure-cpp)

# Update Immer submodule
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/include/immer/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

add_subdirectory(test)
add_subdirectory(doc)
