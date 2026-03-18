cmake_minimum_required(VERSION 3.16)

project(HPUParser C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(hpu_parser
    src/hpu_parser.c
)

target_compile_options(hpu_parser PRIVATE
    -Wall
    -Wextra
    -O2
)