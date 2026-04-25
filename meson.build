project(
  'base',
  ['c', 'cpp'],
  default_options: ['c_std=c17', 'cpp_std=c++20', 'warning_level=3'],
  meson_version: '>=1.10',
)

sources = [
  'src/utilities/assertions.cpp',
  'src/memory/arena.cpp',
  'src/logging/formatter.cpp',
  'src/containers/char_buffer.cpp',
  'src/logging/logger.cpp',
  'src/utilities/terminate.cpp',
  'src/sys/file.cpp',
]

cpp_flags = []
link_flags = []
macros = []

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
cc_id = cc.get_id()
cxx_id = cxx.get_id()

build_dir = meson.project_build_root()
src_dir = meson.project_source_root()

no_libcpp = get_option('no_libcpp')

compiler_cpp_flags = {
  'clang, emscripten': [
    '-Wno-c++98-compat',
    '-Wno-c++98-compat-pedantic',
    '-Wno-unsafe-buffer-usage',
    '-Wno-switch-default',
    '-Wno-padded',
  ],

  'gcc': [
    '-Wno-cast-align',
    '-Wno-padded',
    '-Wno-multiple-inheritance',
    '-Wno-unused-const-variable',
    '-Wno-effc++',
  ],

  f'if @no_libcpp@: clang, emscripten, gcc': [
    '-nostdinc++',
    '-nostdlib++',
  ],
}

foreach compilers, flags : compiler_cpp_flags
  if (compilers.startswith('if false: '))
    continue
  endif
  if (
    compilers == cxx_id
    or compilers == f'if true: @cxx_id@'
    or compilers.startswith(f'@cxx_id@, ')
    or compilers.startswith(f'if true: @cxx_id@, ')
    or compilers.endswith(f', @cxx_id@')
    or compilers.contains(f', @cxx_id@, ')
  )
    cpp_flags += flags
  endif
endforeach

host_system = host_machine.system()
unix = host_system == 'linux' or host_system == 'darwin' or host_system == 'freebsd'

if unix
  macros += 'BASE_PLATFORM_UNIX'
endif

if no_libcpp
  macros += 'BASE_NO_LIBCPP'
endif

macros += 'BASE_COMPILER_' + cxx_id.to_upper()

foreach macro : macros
  cpp_flags += '-D' + macro
endforeach

threads_dep = dependency('threads')

lib = library(
  'base',
  sources: sources,
  include_directories: 'include',
  cpp_args: cpp_flags,
  link_args: link_flags,
  dependencies: threads_dep,
  install: true,
  override_options: {'werror': true, 'warning_level': 'everything'},
)

base_dep = declare_dependency(
  include_directories: 'include',
  link_with: lib,
)

install_subdir('include/base', install_dir: 'include')

pkg = import('pkgconfig')
pkg.generate(lib)

subdir('tests')
