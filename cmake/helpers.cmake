function(add_warnings target)
  if(MSVC)
    target_compile_options("${target}" PRIVATE "/W4")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL
                                                 GNU)
    target_compile_options(
      "${target}"
      PRIVATE "-Wall"
              "-Wextra"
              "-Wpedantic"
              "-Wshadow"
              "-Wundef"
              "-Wformat=2"
              "-Wformat-signedness"
              "-Wmissing-declarations"
              "-Wredundant-decls"
              "-Wuninitialized"
              "-Wpacked"
              "-Wswitch-default"
              "-Wswitch-enum"
              "-Wunused-macros"
              "-Wmissing-include-dirs"
              "-Wstack-protector"
              "-Wvla"
              "-Wdouble-promotion"
              "-Wshift-negative-value"
              "-Walloca"
              "-Wtrivial-auto-var-init"
              "-Wctor-dtor-privacy"
              "-Wnon-virtual-dtor"
              "-Wold-style-cast"
              "-Woverloaded-virtual"
              "-Wsign-promo"
              "-Wzero-as-null-pointer-constant"
              "-Wabi"
              "-Wnoexcept-type"
              "-Wextra-semi"
              "-Wconversion")
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(
      "${target}"
      PRIVATE "-Wconditionally-supported"
              "-Wnoexcept"
              "-Warith-conversion"
              "-Wabi-tag"
              "-Wopenacc-parallelism"
              "-Wunused-const-variable"
              "-Wvector-operation-performance"
              "-Walloc-zero"
              "-Wduplicated-cond"
              "-Wsuggest-attribute=format"
              "-Wsuggest-attribute=noreturn"
              "-Wsuggest-attribute=malloc"
              "-Wno-stringop-overflow"
              "-Wno-array-bounds"
              "-Wlogical-op")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    target_compile_options(
      "${target}"
      PRIVATE "-Wmissing-format-attribute"
              "-Wmissing-noreturn"
              "-Wmissing-noescape"
              # "-Wno-lifetime-safety-all"
              # https://github.com/llvm/llvm-project/issues/221466
              "-Wdangling")
  endif()
endfunction()

function(set_utf8 target)
  if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    target_compile_options(
      "${target}" PUBLIC "$<$<CXX_COMPILER_ID:GNU>:-finput-charset=utf-8>"
                         "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
  endif()
endfunction()

function(set_hidden_visibility target)
  set_target_properties("${target}" PROPERTIES VISIBILITY_INLINES_HIDDEN YES
                                               CXX_VISIBILITY_PRESET hidden)
endfunction()

function(link_threads target)
  find_package(Threads REQUIRED)
  target_link_libraries("${target}" PRIVATE Threads::Threads)
  if(EMSCRIPTEN)
    target_compile_options("${target}" PUBLIC "-pthread")
    target_link_options("${target}" PUBLIC "-pthread")
  endif()
endfunction()

function(gen_export_header target path public_api_macro)
  target_sources(
    "${target}" PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                       FILES "${CMAKE_CURRENT_BINARY_DIR}/${path}")
  include(GenerateExportHeader)
  generate_export_header("${target}" EXPORT_FILE_NAME "${path}"
                         EXPORT_MACRO_NAME "${public_api_macro}")

  target_include_directories(
    "${target}" PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
                       "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
endfunction()

function(public_include_directory target path)
  file(GLOB_RECURSE files "${path}/*.h" "${path}/*.hpp" "${path}/*.hxx")
  target_sources("${target}" PUBLIC FILE_SET HEADERS BASE_DIRS "${path}" FILES
                                    "${files}")

  include(GNUInstallDirs)
  target_include_directories(
    "${target}" PUBLIC "$<BUILD_INTERFACE:${path}>"
                       "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
endfunction()

function(source_directory target path)
  file(GLOB_RECURSE files "${path}/*.c" "${path}/*.cpp" "${path}/*.cxx")
  target_sources("${target}" PRIVATE "${files}")
endfunction()

# This function accepts target names after project
function(install_project_targets project)
  include(CMakePackageConfigHelpers)
  include(GNUInstallDirs)

  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${project}-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${project}-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${project}")

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${project}-config-version.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion)

  install(
    TARGETS "${ARGN}"
    EXPORT "${project}-targets"
    INCLUDES
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            FILE_SET HEADERS
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

  install(
    EXPORT "${project}-targets"
    FILE "${project}-targets.cmake"
    NAMESPACE "${project}::"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${project}")

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${project}-config.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/${project}-config-version.cmake"
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${project}")
endfunction()
