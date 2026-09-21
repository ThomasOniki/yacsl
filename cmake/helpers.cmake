function(init_target target_name)
  set_target_properties("${target_name}" PROPERTIES COMPILE_WARNING_AS_ERROR
                                                    ${BASE_WERROR})
  if(MSVC)
    target_compile_options("${target_name}" PRIVATE "/W4")
  else()
    target_compile_options(
      "${target_name}"
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
      "${target_name}"
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
      "${target_name}"
      PRIVATE "-Wmissing-format-attribute"
              "-Wmissing-noreturn"
              "-Wmissing-noescape"
              # "-Wno-lifetime-safety-all"
              # https://github.com/llvm/llvm-project/issues/221466
              "-Wdangling")
  endif()
endfunction()
