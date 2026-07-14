#pragma once

#include "ast.h"
#include "token_stream.h"

#include <memory>
#include <string_view>
#include <vector>

class Parser {
public:
    Parser(const File &file, std::vector<Token> tokens);

    std::unique_ptr<CompUnit> parse();

private:
    [[nodiscard]] DeclPtr parse_decl();
    [[nodiscard]] std::unique_ptr<VarDecl> parse_const_decl();
    [[nodiscard]] std::unique_ptr<VarDecl> parse_var_decl();
    [[nodiscard]] VarDef parse_var_def(bool constant_context);
    [[nodiscard]] std::unique_ptr<Initializer> parse_initializer(bool constant_context);

    [[nodiscard]] BasicType parse_basic_type(bool allow_void = false);
    [[nodiscard]] std::unique_ptr<FuncDef> parse_func_def();
    [[nodiscard]] std::unique_ptr<FuncParam> parse_func_param();

    [[nodiscard]] std::unique_ptr<BlockStmt> parse_block();
    [[nodiscard]] BlockItem parse_block_item();
    [[nodiscard]] StmtPtr parse_stmt();
    [[nodiscard]] std::unique_ptr<IfStmt> parse_if_stmt();
    [[nodiscard]] std::unique_ptr<WhileStmt> parse_while_stmt();
    [[nodiscard]] std::unique_ptr<ReturnStmt> parse_return_stmt();
    [[nodiscard]] std::unique_ptr<AssignStmt> parse_assign_stmt();
    [[nodiscard]] bool is_assignment_stmt() const;

    [[nodiscard]] ExprPtr parse_exp();
    [[nodiscard]] ExprPtr parse_cond();
    [[nodiscard]] ExprPtr parse_lor_exp();
    [[nodiscard]] ExprPtr parse_land_exp();
    [[nodiscard]] ExprPtr parse_eq_exp();
    [[nodiscard]] ExprPtr parse_rel_exp();
    [[nodiscard]] ExprPtr parse_add_exp(bool allow_logical_not = false);
    [[nodiscard]] ExprPtr parse_mul_exp(bool allow_logical_not);
    [[nodiscard]] ExprPtr parse_unary_exp(bool allow_logical_not);
    [[nodiscard]] ExprPtr parse_primary_exp(bool allow_logical_not);
    [[nodiscard]] ExprPtr parse_const_exp();
    [[nodiscard]] std::unique_ptr<LValueExpr> parse_lvalue_expr();
    [[nodiscard]] std::unique_ptr<CallExpr> parse_call_expr();

    [[nodiscard]] ExprPtr make_binary(BinaryOp op, ExprPtr lhs, ExprPtr rhs) const;
    [[noreturn]] void unsupported_here(std::string_view expected) const;

    const File &file_;
    std::vector<Token> tokens_storage_;
    TokenStream tokens_;
};
