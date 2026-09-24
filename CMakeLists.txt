cmake_minimum_required(VERSION 3.28)
project(base LANGUAGES CXX C)

include("${PROJECT_SOURCE_DIR}/cmake/helpers.cmake")

add_library(base)
add_library(base::base ALIAS base)

source_directory(base "${PROJECT_SOURCE_DIR}/src")
public_include_directory(base "${PROJECT_SOURCE_DIR}/include")
gen_export_header(base "base/export.hpp")

add_warnings(base)
set_utf8(base)
set_hidden_visibility(base)
link_threads(base)

option(BASE_WERROR "Treat warnings as errors." ON)
option(BASE_NO_LIBCPP "Don't link with the c++ standard library." OFF)

set_target_properties(base PROPERTIES COMPILE_WARNING_AS_ERROR ${BASE_WERROR})

if(BASE_NO_LIBCPP)
  target_compile_options(base PUBLIC "-nostdinc++" "-fno-exceptions")
  target_link_options(base PUBLIC "-nostdlib++")
  target_compile_definitions(base PUBLIC BASE_NO_LIBCPP)
endif()

file(GLOB_RECURSE BASE_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
target_sources(base PRIVATE "${BASE_SOURCES}")

target_precompile_headers(base PUBLIC "${PROJECT_SOURCE_DIR}/src/pch.hpp")

target_compile_features(base PUBLIC cxx_std_23)

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
endif()

install_library(base base)
