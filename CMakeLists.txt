cmake_minimum_required(VERSION 3.28)

project(Sysy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 默认 OFF
# 使用 cmake -DUSE_FAST_IO=ON ON
option(USE_FAST_IO "use high performance file reading method (Method 1)")
option(SYSY_PRINT_TOKENS "print lexer tokens")

add_executable(Sysy
    src/main.cpp
    src/compiler/lexer.cpp
    src/compiler/Sysy.cpp
)

target_include_directories(Sysy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler
)

if(USE_FAST_IO)
    target_compile_definitions(Sysy PRIVATE USE_FAST_IO)
endif()

if(SYSY_PRINT_TOKENS)
    target_compile_definitions(Sysy PRIVATE SYSY_PRINT_TOKENS)
endif()

# target_compile_definitions(lexer_model PRIVATE
#     LEXER_TEST
# )
