cmake_minimum_required(VERSION 3.28)
project(yacsl LANGUAGES CXX C)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers.cmake")

add_library(yacsl)
add_library(yacsl::yacsl ALIAS yacsl)

source_directory(yacsl "${CMAKE_CURRENT_SOURCE_DIR}/src")
public_include_directory(yacsl "${CMAKE_CURRENT_SOURCE_DIR}/include")
gen_export_header(yacsl "yacsl/export.hpp" "YACSL_API")

add_warnings(yacsl)
set_utf8(yacsl)
set_hidden_visibility(yacsl)
link_threads(yacsl)
target_compile_features(yacsl PUBLIC cxx_std_23)

option(YACSL_WERROR "Treat warnings as errors." ON)
option(YACSL_NO_LIBCPP "Don't link with the c++ standard library." OFF)

set_target_properties(yacsl PROPERTIES COMPILE_WARNING_AS_ERROR ${YACSL_WERROR})

if(YACSL_NO_LIBCPP)
  target_compile_options(yacsl PUBLIC "-nostdinc++" "-fno-exceptions")
  target_link_options(yacsl PUBLIC "-nostdlib++")
  target_compile_definitions(yacsl PUBLIC YACSL_NO_LIBCPP)
endif()

target_precompile_headers(yacsl PUBLIC
                          "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()

install_library(yacsl yacsl)
