#include "ast_dump.h"

#include <string>
#include <string_view>
#include <type_traits>

namespace {

std::string_view basic_type_name(const BasicType type) {
    switch (type) {
    case BasicType::Void: return "void";
    case BasicType::Int: return "int";
    case BasicType::Float: return "float";
    }
    return "<unknown>";
}

std::string_view binary_op_name(const BinaryOp op) {
    switch (op) {
    case BinaryOp::Add: return "Add";
    case BinaryOp::Sub: return "Sub";
    case BinaryOp::Mul: return "Mul";
    case BinaryOp::Div: return "Div";
    case BinaryOp::Mod: return "Mod";
    case BinaryOp::Lt: return "Lt";
    case BinaryOp::Le: return "Le";
    case BinaryOp::Gt: return "Gt";
    case BinaryOp::Ge: return "Ge";
    case BinaryOp::Eq: return "Eq";
    case BinaryOp::Ne: return "Ne";
    case BinaryOp::LogicalAnd: return "LogicalAnd";
    case BinaryOp::LogicalOr: return "LogicalOr";
    }
    return "<unknown>";
}

std::string_view unary_op_name(const UnaryOp op) {
    switch (op) {
    case UnaryOp::Plus: return "Plus";
    case UnaryOp::Minus: return "Minus";
    case UnaryOp::LogicalNot: return "LogicalNot";
    }
    return "<unknown>";
}

void indent(std::ostream &out, const size_t depth) {
    out << std::string(depth * 2, ' ');
}

void dump_range(const SourceRange range, std::ostream &out) {
    out << " range=[" << range.begin.offset << ", " << range.end.offset << ")";
}

void dump_expr(const Expr &expr, std::ostream &out, size_t depth);
void dump_node(const Node &node, std::ostream &out, size_t depth);

void dump_initializer(const Initializer &initializer, std::ostream &out, const size_t depth) {
    indent(out, depth);
    out << "Initializer";
    dump_range(initializer.range, out);
    out << '\n';
    if (const auto *expression = std::get_if<ExprPtr>(&initializer.value)) {
        dump_expr(**expression, out, depth + 1);
        return;
    }
    for (const auto &element : std::get<std::vector<std::unique_ptr<Initializer>>>(initializer.value)) {
        dump_initializer(*element, out, depth + 1);
    }
}

void dump_expr(const Expr &expr, std::ostream &out, const size_t depth) {
    indent(out, depth);
    switch (expr.kind) {
    case NodeKind::IntLiteral: {
        const auto &node = static_cast<const IntLiteral &>(expr);
        out << "IntLiteral value=" << node.value;
        dump_range(node.range, out);
        out << '\n';
        return;
    }
    case NodeKind::FloatLiteral: {
        const auto &node = static_cast<const FloatLiteral &>(expr);
        out << "FloatLiteral value=" << node.value;
        dump_range(node.range, out);
        out << '\n';
        return;
    }
    case NodeKind::LValueExpr: {
        const auto &node = static_cast<const LValueExpr &>(expr);
        out << "LValueExpr name=" << node.name;
        dump_range(node.range, out);
        out << '\n';
        for (const ExprPtr &index : node.indices) dump_expr(*index, out, depth + 1);
        return;
    }
    case NodeKind::UnaryExpr: {
        const auto &node = static_cast<const UnaryExpr &>(expr);
        out << "UnaryExpr op=" << unary_op_name(node.op);
        dump_range(node.range, out);
        out << '\n';
        dump_expr(*node.operand, out, depth + 1);
        return;
    }
    case NodeKind::BinaryExpr: {
        const auto &node = static_cast<const BinaryExpr &>(expr);
        out << "BinaryExpr op=" << binary_op_name(node.op);
        dump_range(node.range, out);
        out << '\n';
        dump_expr(*node.lhs, out, depth + 1);
        dump_expr(*node.rhs, out, depth + 1);
        return;
    }
    case NodeKind::CallExpr: {
        const auto &node = static_cast<const CallExpr &>(expr);
        out << "CallExpr callee=" << node.callee;
        dump_range(node.range, out);
        out << '\n';
        for (const ExprPtr &argument : node.arguments) dump_expr(*argument, out, depth + 1);
        return;
    }
    default:
        out << "<invalid expression>\n";
        return;
    }
}

void dump_var_decl(const VarDecl &declaration, std::ostream &out, const size_t depth) {
    indent(out, depth);
    out << "VarDecl type=" << basic_type_name(declaration.base_type)
        << " const=" << (declaration.is_const ? "true" : "false");
    dump_range(declaration.range, out);
    out << '\n';
    for (const VarDef &definition : declaration.definitions) {
        indent(out, depth + 1);
        out << "VarDef name=" << definition.name;
        dump_range(definition.range, out);
        out << '\n';
        for (const ExprPtr &dimension : definition.dimensions) dump_expr(*dimension, out, depth + 2);
        if (definition.initializer) dump_initializer(*definition.initializer, out, depth + 2);
    }
}

void dump_stmt(const Stmt &stmt, std::ostream &out, const size_t depth) {
    indent(out, depth);
    switch (stmt.kind) {
    case NodeKind::BlockStmt: {
        const auto &node = static_cast<const BlockStmt &>(stmt);
        out << "BlockStmt";
        dump_range(node.range, out);
        out << '\n';
        for (const BlockItem &item : node.items) {
            std::visit(
                [&](const auto &child) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(child)>, DeclPtr>) {
                        dump_node(*child, out, depth + 1);
                    } else {
                        dump_stmt(*child, out, depth + 1);
                    }
                },
                item);
        }
        return;
    }
    case NodeKind::AssignStmt: {
        const auto &node = static_cast<const AssignStmt &>(stmt);
        out << "AssignStmt";
        dump_range(node.range, out);
        out << '\n';
        dump_expr(*node.target, out, depth + 1);
        dump_expr(*node.value, out, depth + 1);
        return;
    }
    case NodeKind::ExprStmt: {
        const auto &node = static_cast<const ExprStmt &>(stmt);
        out << "ExprStmt";
        dump_range(node.range, out);
        out << '\n';
        dump_expr(*node.expression, out, depth + 1);
        return;
    }
    case NodeKind::EmptyStmt:
        out << "EmptyStmt";
        dump_range(stmt.range, out);
        out << '\n';
        return;
    case NodeKind::IfStmt: {
        const auto &node = static_cast<const IfStmt &>(stmt);
        out << "IfStmt";
        dump_range(node.range, out);
        out << '\n';
        dump_expr(*node.condition, out, depth + 1);
        dump_stmt(*node.then_branch, out, depth + 1);
        if (node.else_branch) dump_stmt(*node.else_branch, out, depth + 1);
        return;
    }
    case NodeKind::WhileStmt: {
        const auto &node = static_cast<const WhileStmt &>(stmt);
        out << "WhileStmt";
        dump_range(node.range, out);
        out << '\n';
        dump_expr(*node.condition, out, depth + 1);
        dump_stmt(*node.body, out, depth + 1);
        return;
    }
    case NodeKind::BreakStmt:
        out << "BreakStmt";
        dump_range(stmt.range, out);
        out << '\n';
        return;
    case NodeKind::ContinueStmt:
        out << "ContinueStmt";
        dump_range(stmt.range, out);
        out << '\n';
        return;
    case NodeKind::ReturnStmt: {
        const auto &node = static_cast<const ReturnStmt &>(stmt);
        out << "ReturnStmt";
        dump_range(node.range, out);
        out << '\n';
        if (node.value) dump_expr(*node.value, out, depth + 1);
        return;
    }
    default:
        out << "<invalid statement>\n";
        return;
    }
}

void dump_node(const Node &node, std::ostream &out, const size_t depth) {
    if (node.kind == NodeKind::VarDecl) {
        dump_var_decl(static_cast<const VarDecl &>(node), out, depth);
        return;
    }
    if (node.kind == NodeKind::FuncDef) {
        const auto &function = static_cast<const FuncDef &>(node);
        indent(out, depth);
        out << "FuncDef name=" << function.name << " return=" << basic_type_name(function.return_type);
        dump_range(function.range, out);
        out << '\n';
        for (const auto &parameter : function.parameters) {
            indent(out, depth + 1);
            out << "FuncParam name=" << parameter->name << " type=" << basic_type_name(parameter->base_type)
                << " array=" << (parameter->is_array ? "true" : "false");
            dump_range(parameter->range, out);
            out << '\n';
            for (const ExprPtr &dimension : parameter->dimensions) dump_expr(*dimension, out, depth + 2);
        }
        dump_stmt(*function.body, out, depth + 1);
        return;
    }
    if (const auto *statement = dynamic_cast<const Stmt *>(&node)) {
        dump_stmt(*statement, out, depth);
        return;
    }
    indent(out, depth);
    out << "<invalid node>\n";
}

} // namespace

void dump_ast(const CompUnit &unit, std::ostream &out) {
    out << "CompUnit";
    dump_range(unit.range, out);
    out << '\n';
    for (const DeclPtr &declaration : unit.declarations) dump_node(*declaration, out, 1);
}
