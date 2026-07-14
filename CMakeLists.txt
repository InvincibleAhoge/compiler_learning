cmake_minimum_required(VERSION 3.28)

project(Sysy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SYSY_PRINT_TOKENS "print lexer tokens")

add_executable(Sysy
    src/main.cpp
    src/compiler/ast.cpp
    src/compiler/ast_dump.cpp
    src/compiler/lexer.cpp
    src/compiler/parser.cpp
    src/compiler/Sysy.cpp
    src/compiler/error.h
    src/compiler/error.cpp
    src/compiler/token_stream.cpp
)

target_include_directories(Sysy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler
)

if(SYSY_PRINT_TOKENS)
    target_compile_definitions(Sysy PRIVATE SYSY_PRINT_TOKENS)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME parser_minimal_main
        COMMAND Sysy --dump-ast ${CMAKE_CURRENT_SOURCE_DIR}/test/parser/minimal_main.sy)
    set_tests_properties(parser_minimal_main PROPERTIES
        PASS_REGULAR_EXPRESSION "CompUnit.*FuncDef name=main return=int.*BlockStmt.*ReturnStmt.*IntLiteral value=0")

    foreach(accessible_case IN ITEMS
            array_init
            fib
            float
            global_var
            nested_if_while)
        add_test(NAME parser_accessible_${accessible_case}
            COMMAND Sysy --dump-ast
                ${CMAKE_CURRENT_SOURCE_DIR}/test/accessible/${accessible_case}.sy)
    endforeach()
    set_tests_properties(parser_accessible_fib PROPERTIES
        PASS_REGULAR_EXPRESSION "FuncParam name=n type=int.*IfStmt.*BinaryExpr op=Le.*CallExpr callee=fib.*BinaryExpr op=Sub")
    set_tests_properties(parser_accessible_array_init PROPERTIES
        PASS_REGULAR_EXPRESSION "VarDecl type=int const=true.*Initializer.*WhileStmt.*AssignStmt")

    add_test(NAME parser_reports_expected_punctuator
        COMMAND Sysy --dump-ast ${CMAKE_CURRENT_SOURCE_DIR}/test/parser/missing_return_semicolon.sy)
    set_tests_properties(parser_reports_expected_punctuator PROPERTIES
        WILL_FAIL TRUE)
endif()

# target_compile_definitions(lexer_model PRIVATE
#     LEXER_TEST
# )
