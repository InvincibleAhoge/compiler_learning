#pragma once

#include "Sysy.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

enum class NodeKind {
    CompUnit,
    VarDecl,
    FuncDef,
    FuncParam,
    BlockStmt,
    AssignStmt,
    ExprStmt,
    EmptyStmt,
    IfStmt,
    WhileStmt,
    BreakStmt,
    ContinueStmt,
    ReturnStmt,
    BinaryExpr,
    UnaryExpr,
    CallExpr,
    LValueExpr,
    IntLiteral,
    FloatLiteral,
};

enum class BasicType {
    Void,
    Int,
    Float,
};

enum class BinaryOp {
    Add, Sub, Mul, Div, Mod,
    Lt, Le, Gt, Ge,
    Eq, Ne,
    LogicalAnd, LogicalOr,
};

enum class UnaryOp {
    Plus,
    Minus,
    LogicalNot,
};

struct Node {
    explicit Node(NodeKind kind, SourceRange range) : kind(kind), range(range) {}
    virtual ~Node() = default;

    NodeKind kind;
    SourceRange range;
};

struct Expr : Node {
    Expr(NodeKind kind, SourceRange range) : Node(kind, range) {}
};

struct Stmt : Node {
    Stmt(NodeKind kind, SourceRange range) : Node(kind, range) {}
};

struct Decl : Node {
    Decl(NodeKind kind, SourceRange range) : Node(kind, range) {}
};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;
using BlockItem = std::variant<DeclPtr, StmtPtr>;

struct IntLiteral final : Expr {
    IntLiteral(SourceRange range, sysy::i32 value)
        : Expr(NodeKind::IntLiteral, range), value(value) {}

    sysy::i32 value;
};

struct FloatLiteral final : Expr {
    FloatLiteral(SourceRange range, sysy::f32 value)
        : Expr(NodeKind::FloatLiteral, range), value(value) {}

    sysy::f32 value;
};

struct LValueExpr final : Expr {
    LValueExpr(SourceRange range, std::string name)
        : Expr(NodeKind::LValueExpr, range), name(std::move(name)) {}

    std::string name;
    std::vector<ExprPtr> indices;
};

struct UnaryExpr final : Expr {
    UnaryExpr(SourceRange range, UnaryOp op, ExprPtr operand)
        : Expr(NodeKind::UnaryExpr, range), op(op), operand(std::move(operand)) {}

    UnaryOp op;
    ExprPtr operand;
};

struct BinaryExpr final : Expr {
    BinaryExpr(SourceRange range, BinaryOp op, ExprPtr lhs, ExprPtr rhs)
        : Expr(NodeKind::BinaryExpr, range), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    BinaryOp op;
    ExprPtr lhs;
    ExprPtr rhs;
};

struct CallExpr final : Expr {
    CallExpr(SourceRange range, std::string callee)
        : Expr(NodeKind::CallExpr, range), callee(std::move(callee)) {}

    std::string callee;
    std::vector<ExprPtr> arguments;
};

struct Initializer {
    explicit Initializer(SourceRange range, ExprPtr expression)
        : range(range), value(std::move(expression)) {}
    explicit Initializer(SourceRange range, std::vector<std::unique_ptr<Initializer>> elements)
        : range(range), value(std::move(elements)) {}

    SourceRange range;
    std::variant<ExprPtr, std::vector<std::unique_ptr<Initializer>>> value;
};

struct VarDef {
    SourceRange range;
    std::string name;
    std::vector<ExprPtr> dimensions;
    std::unique_ptr<Initializer> initializer;
};

struct VarDecl final : Decl {
    VarDecl(SourceRange range, BasicType base_type, bool is_const)
        : Decl(NodeKind::VarDecl, range), base_type(base_type), is_const(is_const) {}

    BasicType base_type;
    bool is_const;
    std::vector<VarDef> definitions;
};

struct FuncParam final : Decl {
    FuncParam(SourceRange range, BasicType base_type, std::string name)
        : Decl(NodeKind::FuncParam, range), base_type(base_type), name(std::move(name)) {}

    BasicType base_type;
    std::string name;
    bool is_array = false;
    std::vector<ExprPtr> dimensions;
};

struct BlockStmt final : Stmt {
    explicit BlockStmt(SourceRange range) : Stmt(NodeKind::BlockStmt, range) {}

    std::vector<BlockItem> items;
};

struct AssignStmt final : Stmt {
    AssignStmt(SourceRange range, std::unique_ptr<LValueExpr> target, ExprPtr value)
        : Stmt(NodeKind::AssignStmt, range), target(std::move(target)), value(std::move(value)) {}

    std::unique_ptr<LValueExpr> target;
    ExprPtr value;
};

struct ExprStmt final : Stmt {
    ExprStmt(SourceRange range, ExprPtr expression)
        : Stmt(NodeKind::ExprStmt, range), expression(std::move(expression)) {}

    ExprPtr expression;
};

struct EmptyStmt final : Stmt {
    explicit EmptyStmt(SourceRange range) : Stmt(NodeKind::EmptyStmt, range) {}
};

struct IfStmt final : Stmt {
    explicit IfStmt(SourceRange range, ExprPtr condition, StmtPtr then_branch)
        : Stmt(NodeKind::IfStmt, range), condition(std::move(condition)),
          then_branch(std::move(then_branch)) {}

    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;
};

struct WhileStmt final : Stmt {
    WhileStmt(SourceRange range, ExprPtr condition, StmtPtr body)
        : Stmt(NodeKind::WhileStmt, range), condition(std::move(condition)), body(std::move(body)) {}

    ExprPtr condition;
    StmtPtr body;
};

struct BreakStmt final : Stmt {
    explicit BreakStmt(SourceRange range) : Stmt(NodeKind::BreakStmt, range) {}
};

struct ContinueStmt final : Stmt {
    explicit ContinueStmt(SourceRange range) : Stmt(NodeKind::ContinueStmt, range) {}
};

struct ReturnStmt final : Stmt {
    explicit ReturnStmt(SourceRange range) : Stmt(NodeKind::ReturnStmt, range) {}

    ExprPtr value;
};

struct FuncDef final : Decl {
    FuncDef(SourceRange range, BasicType return_type, std::string name)
        : Decl(NodeKind::FuncDef, range), return_type(return_type), name(std::move(name)) {}

    BasicType return_type;
    std::string name;
    std::vector<std::unique_ptr<FuncParam>> parameters;
    std::unique_ptr<BlockStmt> body;
};

struct CompUnit final : Node {
    explicit CompUnit(SourceRange range) : Node(NodeKind::CompUnit, range) {}

    std::vector<DeclPtr> declarations;
};
