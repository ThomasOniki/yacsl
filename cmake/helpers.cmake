function(init_target target_name)
  set_target_properties("${target_name}" PROPERTIES COMPILE_WARNING_AS_ERROR
                                                    ${BASE_WERROR})
  target_compile_options("${target_name}" PRIVATE "${BASE_PRIVATE_FLAGS}")
endfunction()

if(MSVC)
  set(BASE_PRIVATE_FLAGS "/W4")
else()
  set(BASE_PRIVATE_FLAGS
      "-Wall"
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
      "-Wconversion"
      "$<$<CXX_COMPILER_ID:GNU>:-Wconditionally-supported>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wnoexcept>"
      "$<$<CXX_COMPILER_ID:GNU>:-Warith-conversion>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wabi-tag>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wopenacc-parallelism>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wunused-const-variable>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wvector-operation-performance>"
      "$<$<CXX_COMPILER_ID:GNU>:-Walloc-zero>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wsuggest-attribute=format>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wsuggest-attribute=noreturn>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wsuggest-attribute=malloc>"
      "$<$<CXX_COMPILER_ID:Clang>:-Wmissing-format-attribute>"
      "$<$<CXX_COMPILER_ID:Clang>:-Wmissing-noreturn>"
      "$<$<CXX_COMPILER_ID:Clang>:-Wmissing-noescape>"
      # "-Wno-lifetime-safety-all"
      # https://github.com/llvm/llvm-project/issues/221466
      "$<$<CXX_COMPILER_ID:Clang>:-Wdangling>"
      "$<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>")
endif()
