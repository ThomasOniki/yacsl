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

host_system = host_machine.system()
is_windows = host_system == 'windows'

if cxx_id == 'clang' or cxx_id == 'emscripten'
  lifetimebound_attribute = '[[clang::lifetimebound]]'
else
  lifetimebound_attribute = ''
endif

potential_cpp_flags += [
  'true clang, emscripten',
  [
    '-Wno-c++98-compat',
    '-Wno-c++98-compat-pedantic',
    '-Wno-unsafe-buffer-usage',
    '-Wno-switch-default',
    '-Wno-padded',
  ],
  'true emscripten',
  ['-Wno-disabled-macro-expansion', '-pthread'],
  'true gcc',
  [
    '-Wno-cast-align',
    '-Wno-padded',
    '-Wno-multiple-inheritance',
    '-Wno-unused-const-variable',
    '-Wno-effc++',
  ],
  f'pub @no_libcpp@',
  ['-DBASE_NO_LIBCPP'],
  f'pub @no_libcpp@ clang, emscripten, gcc',
  [
    '-nostdinc++',
    '-nostdlib++',
  ],
  f'pub @is_windows@',
  ['-DBASE_PLATFORM_WINDOWS'],
  'pub true',
  [
    '-DBASE_COMPILER_' + cxx_id.to_upper(), f'-DBASE_LIFETIMEBOUND=@lifetimebound_attribute@',
  ],
]

add_private = false
add_public = false
index = 0
foreach elem : potential_cpp_flags
  if (index % 2 == 0)
    add_private = elem.startswith('true') and (
      elem == 'true'
      or elem.contains(f' @cxx_id@, ')
      or elem.endswith(f' @cxx_id@')
    )
    add_public = elem.startswith('pub true') and (
      elem == 'pub true'
      or elem.contains(f' @cxx_id@, ')
      or elem.endswith(f' @cxx_id@')
    )
  else
    foreach flag : elem
      if add_private
        private_cpp_flags += elem
      endif
      if add_public
        public_cpp_flags += elem
      endif
    endforeach
  endif
  index += 1
endforeach

subdir('base')

if not meson.is_subproject()
  subdir('tests')
endif
