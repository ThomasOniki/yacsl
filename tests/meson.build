if cxx_id == 'clang' or cxx_id == 'emscripten'
  cpp_flags += [
    '-Wno-global-constructors',
    '-Wno-exit-time-destructors',
  ]
endif

if cxx_id == 'gcc'
  cpp_flags += ['-Wno-padded', '-Wno-abi-tag', '-Wno-cast-align']
endif

things_to_test = [
  'logging',
  'function',
  'arena',
  'threads',
]

if cxx_id != 'emscripten'
  things_to_test += 'process'
endif
if no_libcpp
  things_to_test += 'vector'
endif

foreach thing : things_to_test
  exe = executable(
    f'test_@thing@',
    sources: f'src/test_@thing@.cpp',
    dependencies: [base_dep],
    cpp_args: cpp_flags,
    override_options: {'warning_level': 'everything', 'werror': true},
  )
  test(thing, exe)
endforeach
