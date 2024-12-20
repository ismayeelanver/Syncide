# CMake configuration for the Syncide project

cmake_minimum_required(VERSION 3.20)
project(Syncide VERSION 1 LANGUAGES CXX)

# Set the C++ standard and other compiler options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define source files
set(SRC_FILES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/typeChecker/scopes.cpp
)

# Add include directories
include_directories(src/include /usr/lib/llvm-19/include)

# Add executable target
add_executable(syncate ${SRC_FILES})

# Add compile options
target_compile_options(syncate PRIVATE
    -fno-exceptions
    -funwind-tables
    -D_GNU_SOURCE
    -D__STDC_CONSTANT_MACROS
    -D__STDC_FORMAT_MACROS
    -D__STDC_LIMIT_MACROS
)

# Link dependencies
find_package(fmt REQUIRED)
target_link_libraries(syncate PRIVATE fmt)

# Add LLVM library manually
target_link_options(syncate PRIVATE
    -L/usr/lib/llvm-19/lib
    -lLLVM-19
)
