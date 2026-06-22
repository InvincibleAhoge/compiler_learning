cmake_minimum_required(VERSION 3.28)

project(Sysy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SYSY_PRINT_TOKENS "print lexer tokens")

add_executable(Sysy
    src/main.cpp
    src/compiler/lexer.cpp
    src/compiler/Sysy.cpp
    src/compiler/error.h
    src/compiler/error.cpp
)

target_include_directories(Sysy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler
)

if(SYSY_PRINT_TOKENS)
    target_compile_definitions(Sysy PRIVATE SYSY_PRINT_TOKENS)
endif()

# target_compile_definitions(lexer_model PRIVATE
#     LEXER_TEST
# )
