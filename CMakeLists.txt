cmake_minimum_required(VERSION 3.20)

project(HPUEncode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(hpu_encode
    src/instruction.cpp
    src/parser.cpp
    src/encoder.cpp
    src/assembler.cpp
)

target_include_directories(hpu_encode
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(hpu_encode PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

add_executable(hpu_encode_cli src/main.cpp)
target_link_libraries(hpu_encode_cli PRIVATE hpu_encode)

target_compile_options(hpu_encode_cli PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

include(CTest)
option(HPU_ENABLE_TESTS "Build GoogleTest-based tests" ON)

if(BUILD_TESTING AND HPU_ENABLE_TESTS)
    add_executable(asm_file_dump tests/asm_file_dump.cpp)
    target_link_libraries(asm_file_dump PRIVATE hpu_encode)
    target_compile_options(asm_file_dump PRIVATE -Wall -Wextra -Wpedantic -O2)

    include(FetchContent)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)
    add_test(NAME hpu_encode_cli_smoke
        COMMAND hpu_encode_cli ${CMAKE_CURRENT_SOURCE_DIR}/formatted_hpu_examples.asm)

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*_test.cpp)

    foreach(test_source IN LISTS TEST_SOURCES)
        get_filename_component(test_name ${test_source} NAME_WE)
        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} PRIVATE hpu_encode GTest::gtest_main)
        target_compile_options(${test_name} PRIVATE -Wall -Wextra -Wpedantic -O2)
        gtest_discover_tests(${test_name})
    endforeach()
endif()
