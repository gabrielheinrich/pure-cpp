find_package (Doxygen)
if (NOT DOXYGEN_FOUND)
    message (STATUS "Doxygen not found. docs won't be build")
else() 
    add_custom_target (doc
        ${DOXYGEN_EXECUTABLE} Doxygen.config
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
        )
endif()
