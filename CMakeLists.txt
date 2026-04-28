cmake_minimum_required(VERSION 3.28)

project(compiler_learning LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lexer_model
    main.cpp
    src/lexer.cpp
    src/reader.cpp
    src/lexer.h
    src/reader.h
)

target_include_directories(lexer_model PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(lexer_model PRIVATE
    LEXER_TEST
)