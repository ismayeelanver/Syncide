project('Syncide', 'cpp', version: '1', default_options: ['cpp_std=c++23'])

src = ['src/main.cpp', 'src/lexer.cpp', 'src/parser.cpp', 'src/typeChecker/scopes.cpp']

dependencies = [dependency('fmt')]

executable(
 'syncate',
 src,
 include_directories: 'src/include',
 dependencies: dependencies,
 cpp_args: [
  '-I/usr/lib/llvm-19/include',
  '-std=c++17',
  '-fno-exceptions',
  '-funwind-tables',
  '-D_GNU_SOURCE',
  '-D__STDC_CONSTANT_MACROS',
  '-D__STDC_FORMAT_MACROS',
  '-D__STDC_LIMIT_MACROS',
  '-L/usr/lib/llvm-19/lib',
  '-lLLVM-19',
 ],
)