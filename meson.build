project(
  'base',
  ['c', 'cpp'],
  default_options: ['c_std=c17', 'cpp_std=c++20', 'warning_level=3'],
  meson_version: '>=1.10',
)

sources = ['src/utils/assertions.cpp', 'src/memory/arena.cpp']

flags = []
link_flags = []
macros = []

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
cc_id = cc.get_id()
cxx_id = cxx.get_id()

host_system = host_machine.system()
unix = host_system == 'linux' or host_system == 'darwin' or host_system == 'freebsd'

cc_clang_or_gcc = cc_id == 'clang' or cc_id == 'gcc'
cxx_clang_or_gcc = cxx_id == 'clang' or cxx_id == 'gcc'
clang_or_gcc = cc_clang_or_gcc or cxx_clang_or_gcc

build_dir = meson.project_build_root()
src_dir = meson.project_source_root()

export_compile_commands = get_option('lsp') or (not get_option('not_lsp') and build_dir == src_dir / 'lsp-build')

no_libcpp = get_option('no_libcpp') or export_compile_commands

if clang_or_gcc
  flags += [
    '-Weverything',
    '-Wno-c++98-compat',
    '-Wno-unsafe-buffer-usage',
  ]
  if no_libcpp
    flags += '-nostdinc++'
    link_flags += '-nostdlib++'
  endif

endif

if unix
  macros += 'BASE_PLATFORM_UNIX'
endif

if no_libcpp
  macros += 'BASE_NO_LIBCPP'
endif

if export_compile_commands
  macros += 'BASE_LSP_ONLY'
endif

foreach macro : macros
  flags += '-D' + macro
endforeach

threads_dep = dependency('threads')

lib = library(
  'base',
  sources: sources,
  include_directories: 'include',
  c_args: flags,
  cpp_args: flags,
  link_args: link_flags,
  dependencies: threads_dep,
  install: true,
)

base_dep = declare_dependency(
  include_directories: 'include',
  link_with: lib,
)

install_subdir('include/base', install_dir: 'include')

pkg = import('pkgconfig')
pkg.generate(lib)

if not no_libcpp or export_compile_commands
  subdir('tests')
endif
