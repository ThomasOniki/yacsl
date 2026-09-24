include(CTest)

file(GLOB_RECURSE BASE_TESTS "${PROJECT_SOURCE_DIR}/tests/src/*.cpp")
foreach(path IN LISTS BASE_TESTS)
  get_filename_component(name "${path}" NAME_WE)

  if("${name}" STREQUAL "test_vector" AND BASE_NO_LIBCPP)
    continue()
  endif()

  message(STATUS "Added test ${name} (${path})")

  add_executable("${name}" "${path}")
  add_warnings("${name}")

  target_compile_definitions(
    "${name}" PRIVATE "BASE_TESTS_DIR=\"${PROJECT_SOURCE_DIR}/tests\"")

  target_link_libraries("${name}" PRIVATE base)
  add_test(NAME "${name}" COMMAND "${name}")
endforeach()

add_executable(test_process_cat
               "${PROJECT_SOURCE_DIR}/tests/share/test_process/cat.c")
add_executable(test_process_std_streams
               "${PROJECT_SOURCE_DIR}/tests/share/test_process/std_streams.c")
add_executable(test_process_hello_world
               "${PROJECT_SOURCE_DIR}/tests/share/test_process/hello_world.c")

target_compile_definitions(
  test_process
  PRIVATE "CAT_EXECUTABLE=\"$<TARGET_FILE:test_process_cat>\""
          "STD_STREAMS_EXECUTABLE=\"$<TARGET_FILE:test_process_std_streams>\""
          "HELLO_WORLD_EXECUTABLE=\"$<TARGET_FILE:test_process_hello_world>\"")
