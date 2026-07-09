cmake_minimum_required(VERSION 3.28)
project(base LANGUAGES CXX)

option(BASE_STATIC_LIB "Build a static library." ON)
option(BASE_SHARED_LIB "Build a shared library." OFF)
option(BASE_WERROR "Treat warnings as errors." ON)
option(BASE_NO_LIBCPP "Don't link with the c++ standard library." OFF)

include("${PROJECT_SOURCE_DIR}/cmake/helpers.cmake")

add_library(base)

init_target(base)

if(BASE_NO_LIBCPP)
  target_compile_options(base PUBLIC "-nostdlib++" "-nostdinc++")
  target_compile_definitions(base PUBLIC BASE_NO_LIBCPP)
endif()

file(GLOB_RECURSE BASE_MODULE_INTERFACES "${PROJECT_SOURCE_DIR}/src/*.cppm")
target_sources(base PUBLIC FILE_SET CXX_MODULES FILES
                           "${BASE_MODULE_INTERFACES}")

file(GLOB_RECURSE BASE_HEADERS "${PROJECT_SOURCE_DIR}/include/*.hpp")
target_sources(base PUBLIC FILE_SET HEADERS FILES "${BASE_HEADERS}")

include(GNUInstallDirs)
target_include_directories(
  base PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
              "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

file(GLOB_RECURSE BASE_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
target_sources(base PRIVATE "${BASE_SOURCES}")

target_compile_features(base PUBLIC cxx_std_20)

set_target_properties(base PROPERTIES VISIBILITY_INLINES_HIDDEN YES
                                      CXX_VISIBILITY_PRESET hidden)

find_package(Threads REQUIRED)
target_link_libraries(base PUBLIC Threads::Threads)

if(EMSCRIPTEN)
  target_compile_options(base PUBLIC "-pthread")
  target_link_options(base PUBLIC "-pthread")
endif()

include(GenerateExportHeader)
generate_export_header(base EXPORT_FILE_NAME "base_export.hpp")

target_sources(
  base
  PUBLIC FILE_SET
         generated_headers
         TYPE
         HEADERS
         BASE_DIRS
         "${CMAKE_CURRENT_BINARY_DIR}"
         FILES
         "${CMAKE_CURRENT_BINARY_DIR}/base_export.hpp")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/base-config.cmake.in"
  "${PROJECT_BINARY_DIR}/base-config.cmake" INSTALL_DESTINATION lib/cmake/base)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/base-config-version.cmake"
  VERSION 1.0.0
  COMPATIBILITY SameMajorVersion)

install(
  TARGETS base
  EXPORT base-targets
  INCLUDES
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
          FILE_SET CXX_MODULES
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/src"
          FILE_SET HEADERS
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
          FILE_SET generated_headers
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
  EXPORT base-targets
  FILE base-targets.cmake
  NAMESPACE base::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/base")

install(FILES "${PROJECT_BINARY_DIR}/base-config.cmake"
              "${PROJECT_BINARY_DIR}/base-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/base")

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
endif()
