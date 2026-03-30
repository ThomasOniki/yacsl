if cxx_id == 'clang' or cxx_id == 'emscripten'
  cpp_flags += [
    '-Wno-global-constructors',
    '-Wno-exit-time-destructors',
  ]
endif

gtest_flags = ''

if cxx_id == 'emscripten'
  cpp_flags += '-pthread'
  gtest_flags += '-pthread'
endif

if cxx_id == 'gcc'
  cpp_flags += ['-Wno-padded', '-Wno-abi-tag', '-Wno-cast-align']
endif

cmake = import('cmake')

gtest_opts = cmake.subproject_options()

if gtest_flags != ''
  gtest_opts.append_compile_args('cpp', gtest_flags)
endif

gtest_proj = cmake.subproject('googletest', options: gtest_opts)

gtest_dep = gtest_proj.dependency('gtest')
gtest_main_dep = gtest_proj.dependency('gtest_main')

arena = executable(
  'test_arena',
  sources: 'src/test_arena.cpp',
  dependencies: [base_dep, gtest_dep, gtest_main_dep],
  cpp_args: cpp_flags,
  override_options: {'warning_level': 'everything', 'werror': true},
)
vector = executable(
  'test_vector',
  sources: 'src/test_vector.cpp',
  dependencies: [base_dep, gtest_dep, gtest_main_dep],
  cpp_args: cpp_flags,
  override_options: {'warning_level': 'everything', 'werror': true},
)

test('arena', arena)
test('vector', vector)
