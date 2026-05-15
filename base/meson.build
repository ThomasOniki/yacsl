sources = [
  'src/utilities/assertions.cpp',
  'src/memory/arena.cpp',
  'src/logging/formatter.cpp',
  'src/containers/char_buffer.cpp',
  'src/logging/logger.cpp',
  'src/utilities/terminate.cpp',
  'src/sys/file.cpp',
  'src/sys/process.cpp',
  'src/sys/errno.cpp',
]
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
  cpp_pch: 'src/pch.hpp',
)
base_dep = declare_dependency(
  include_directories: 'include',
  link_with: lib,
)
install_subdir('include/base', install_dir: 'include')
pkg = import('pkgconfig')
pkg.generate(lib)
