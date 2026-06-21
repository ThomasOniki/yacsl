add_library(base)

init_target(base)

file(GLOB_RECURSE BASE_MODULE_INTERFACES
     "${PROJECT_SOURCE_DIR}/base/src/*.cppm")
target_sources(base PUBLIC FILE_SET CXX_MODULES FILES
                           "${BASE_MODULE_INTERFACES}")

file(GLOB_RECURSE BASE_HEADERS "${PROJECT_SOURCE_DIR}/base/include/*.hpp")
target_sources(base PUBLIC FILE_SET HEADERS FILES "${BASE_HEADERS}")
target_include_directories(base PRIVATE "${PROJECT_SOURCE_DIR}/base/include")

file(GLOB_RECURSE BASE_SOURCES "${PROJECT_SOURCE_DIR}/base/src/*.cpp")
target_sources(base PRIVATE "${BASE_SOURCES}")

target_compile_features(base PUBLIC cxx_std_20)

set_target_properties(base PROPERTIES VISIBILITY_INLINES_HIDDEN YES
                                      CXX_VISIBILITY_PRESET hidden)

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

include(GNUInstallDirs)

install(
  TARGETS base
  EXPORT base-targets
  INCLUDES
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
          FILE_SET CXX_MODULES
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/base/src"
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
