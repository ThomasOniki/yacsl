cmake_minimum_required(VERSION 3.28)
project(base LANGUAGES CXX)

option(BASE_STATIC_LIB "Build a static library." ON)
option(BASE_SHARED_LIB "Build a shared library." OFF)
option(BASE_WERROR "Treat warnings as errors." ON)
option(BASE_NO_LIBCPP "Don't link with the c++ standard library." OFF)

include("${PROJECT_SOURCE_DIR}/cmake/helpers.cmake")

add_subdirectory("${PROJECT_SOURCE_DIR}/base")

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
endif()
