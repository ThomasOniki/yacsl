if cxx_id == 'clang' or cxx_id == 'emscripten'
  cpp_flags += [
    '-Wno-global-constructors',
    '-Wno-exit-time-destructors',
  ]
endif

if cxx_id == 'gcc'
  cpp_flags += ['-Wno-padded', '-Wno-abi-tag', '-Wno-cast-align']
endif

things_to_test = {
  'true': [
    'logging',
    'function',
    'arena',
    'threads',
    'process',
  ],
  f'!@no_libcpp@': [
    'vector',
  ],
}

foreach condition, things : things_to_test
  if condition == 'false' or condition == '!true'
    continue
  endif
  foreach thing : things
    exe = executable(
      f'test_@thing@',
      sources: f'src/test_@thing@.cpp',
      dependencies: [base_dep],
      cpp_args: cpp_flags,
      override_options: {'warning_level': 'everything', 'werror': true},
    )
    test(thing, exe)
  endforeach
endforeach
