project(
  'base',
  'cpp',
  default_options: ['c_std=c17', 'cpp_std=c++20', 'warning_level=3'],
  meson_version: '>=1.10',
)

private_cpp_flags = []
public_cpp_flags = []
link_flags = []

cxx = meson.get_compiler('cpp')
cxx_id = cxx.get_id()

build_dir = meson.project_build_root()
src_dir = meson.project_source_root()

no_libcpp = get_option('no_libcpp')

potential_cpp_flags = []

potential_cpp_flags += [
  'true clang, emscripten',
  '-Wno-c++98-compat',
  '-Wno-c++98-compat-pedantic',
  '-Wno-unsafe-buffer-usage',
  '-Wno-switch-default',
  '-Wno-padded',
]
potential_cpp_flags += [
  'true emscripten',
  '-Wno-disabled-macro-expansion',
  '-pthread', 'true gcc',
  '-Wno-cast-align',
  '-Wno-padded',
  '-Wno-multiple-inheritance',
  '-Wno-unused-const-variable',
  '-Wno-effc++',
]
potential_cpp_flags += [
  f'pub @no_libcpp@',
  '-DBASE_NO_LIBCPP', f'pub @no_libcpp@ clang, emscripten, gcc',
  '-nostdinc++',
  '-nostdlib++',
]

host_system = host_machine.system()
is_unix = host_system == 'linux' or host_system == 'darwin' or host_system == 'freebsd'
potential_cpp_flags += [
  f'pub @is_unix@',
  '-DBASE_PLATFORM_UNIX',
]

if cxx_id == 'clang' or cxx_id == 'emscripten'
  lifetimebound_attribute = '[[clang::lifetimebound]]'
else
  lifetimebound_attribute = ''
endif
potential_cpp_flags += [
  'pub true',
  '-DBASE_COMPILER_' + cxx_id.to_upper(), f'-DBASE_LIFETIMEBOUND=@lifetimebound_attribute@',
]

add_private = false
add_public = false
foreach flag : potential_cpp_flags
  if (
    flag.startswith('true')
    or flag.startswith('false')
    or flag.startswith('pub')
  )
    add_private = flag.startswith('true') and (
      flag == 'true'
      or flag.contains(f' @cxx_id@, ')
      or flag.endswith(f' @cxx_id@')
    )
    add_public = flag.startswith('pub true') and (
      flag == 'pub true'
      or flag.contains(f' @cxx_id@, ')
      or flag.endswith(f' @cxx_id@')
    )
  else
    if add_private
      private_cpp_flags += flag
    endif
    if add_public
      public_cpp_flags += flag
    endif
  endif
endforeach

subdir('base')

if not meson.is_subproject()
  subdir('tests')
endif
