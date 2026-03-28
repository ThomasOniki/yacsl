test_dep = dependency('gtest')
gtest_main_dep = dependency('gtest_main')

arena = executable(
  'test_arena',
  'src/test_arena.cpp',
  dependencies: [base_dep, gtest_dep, gtest_main_dep],
  cpp_args: '-Wno-all',
)
vector = executable(
  'test_vector',
  'src/test_vector.cpp',
  dependencies: [base_dep, gtest_dep, gtest_main_dep],
  cpp_args: '-Wno-all',
)

test('arena', arena)
test('vector', vector)
