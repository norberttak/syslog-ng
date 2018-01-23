add_unit_test(CRITERION TARGET test_secret_storage DEPENDS secret-storage)
add_unit_test(CRITERION TARGET test_nondumpable_allocator DEPENDS secret-storage)

add_executable(meant_to_be_crashed meant_to_be_crashed.c)
target_link_libraries(meant_to_be_crashed secret-storage)
target_include_directories(meant_to_be_crashed PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories(meant_to_be_crashed PUBLIC "${GLIB_INCLUDE_DIRS}")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/check-secret-core.py
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME check-core-file
  WORKING_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}"
  COMMAND ${PYTHON_EXECUTABLE} check-secret-core.py)
