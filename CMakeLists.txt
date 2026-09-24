cmake_minimum_required(VERSION 3.28)
project(base LANGUAGES CXX C)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers.cmake")

add_library(base)
add_library(base::base ALIAS base)

source_directory(base "${CMAKE_CURRENT_SOURCE_DIR}/src")
public_include_directory(base "${CMAKE_CURRENT_SOURCE_DIR}/include")
gen_export_header(base "base/export.hpp" "BASE_API")

add_warnings(base)
set_utf8(base)
set_hidden_visibility(base)
link_threads(base)
target_compile_features(base PUBLIC cxx_std_23)

option(BASE_WERROR "Treat warnings as errors." ON)
option(BASE_NO_LIBCPP "Don't link with the c++ standard library." OFF)

set_target_properties(base PROPERTIES COMPILE_WARNING_AS_ERROR ${BASE_WERROR})

if(BASE_NO_LIBCPP)
  target_compile_options(base PUBLIC "-nostdinc++" "-fno-exceptions")
  target_link_options(base PUBLIC "-nostdlib++")
  target_compile_definitions(base PUBLIC BASE_NO_LIBCPP)
endif()

target_precompile_headers(base PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()

install_library(base base)
