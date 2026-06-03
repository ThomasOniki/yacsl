sources = [
  'src/utilities/assertions.cpp',
  'src/memory/arena.cpp',
  'src/logging/formatter.cpp',
  'src/containers/char_buffer.cpp',
  'src/logging/logger.cpp',
  'src/logging/panic.cpp',
  'src/sys/unix-file.cpp',
  'src/sys/unix-process.cpp',
  'src/sys/win32-file.cpp',
  'src/sys/win32-process.cpp',
]
threads_dep = dependency('threads')
lib = library(
  'base',
  sources: sources,
  include_directories: 'include',
  cpp_args: public_cpp_flags + private_cpp_flags,
  link_args: link_flags,
  dependencies: threads_dep,
  install: true,
  override_options: {'werror': true, 'warning_level': 'everything'},
  cpp_pch: 'src/pch.hpp',
)
base_dep = declare_dependency(
  include_directories: 'include',
  link_with: lib,
  compile_args: public_cpp_flags,
)
install_subdir('include/base', install_dir: 'include')
pkg = import('pkgconfig')
pkg.generate(lib, extra_cflags: public_cpp_flags)
