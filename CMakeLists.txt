cmake_minimum_required(VERSION 3.28)

project(Sysy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 默认 OFF
# 使用 cmake -DUSE_FAST_IO=ON 可启用 option
option(USE_FAST_IO "use high performance file reading method (Method 1)")

add_executable(Sysy
    src/main.cpp
    src/compiler/lex.cpp
)

target_include_directories(Sysy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler
)

if(DUSE_FAST_IO)
  target_compile_definitions(compiler PRIVATE USE_FAST_IO)
endif()
