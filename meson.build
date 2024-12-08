project('Syncide', 'cpp', version: '1', default_options: ['cpp_std=c++23'])


src = ['src/main.cpp', 'src/lexer.cpp', 'src/parser.cpp']

dependencies = [dependency('fmt')]

executable('syncate', src, include_directories: 'src/include', dependencies: dependencies)
