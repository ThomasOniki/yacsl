include(CTest)

file(GLOB_RECURSE BASE_TESTS "${PROJECT_SOURCE_DIR}/tests/*.cpp")
foreach(path IN LISTS BASE_TESTS)
  get_filename_component(name "${path}" NAME_WE)

  message(STATUS "Added test ${name} (${path})")

  add_executable("${name}" "${path}")
  init_target("${name}")

  target_link_libraries("${name}" PRIVATE base)
  add_test(NAME "${name}" COMMAND "${name}")
endforeach()
