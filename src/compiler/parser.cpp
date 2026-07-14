#include "parser.h"

#include "error.h"

#include <string>
#include <string_view>
#include <utility>

namespace {

bool is_basic_type_keyword(const TokenStream &tokens) {
    return tokens.check_keyword("int") || tokens.check_keyword("float") ||
           tokens.check_keyword("void");
}

bool starts_function_definition(const File &file, const TokenStream &tokens) {
    return is_basic_type_keyword(tokens) && tokens.peek(1).kind == TokenKind::IDENT &&
           tokens.peek(2).kind == TokenKind::PUNCT && lexeme(file, tokens.peek(2)) == "(";
}

} // namespace

Parser::Parser(const File &file, std::vector<Token> tokens)
    : file_(file), tokens_storage_(std::move(tokens)), tokens_(file_, tokens_storage_) {}

std::unique_ptr<CompUnit> Parser::parse() {
    const SourceLoc begin = tokens_.current().loc;
    auto unit = std::make_unique<CompUnit>(SourceRange{begin, begin});

    while (!tokens_.eof()) {
        if (tokens_.check_keyword("const") ||
            (is_basic_type_keyword(tokens_) && !tokens_.check_keyword("void") &&
             !starts_function_definition(file_, tokens_))) {
            unit->declarations.push_back(parse_decl());
            continue;
        }

        if (is_basic_type_keyword(tokens_)) {
            unit->declarations.push_back(parse_func_def());
            continue;
        }

        unsupported_here("a declaration or function definition");
    }

    const Token &eof = tokens_.expect(TokenKind::TK_EOF);
    unit->range = SourceRange{begin, eof.loc};
    return unit;
}

DeclPtr Parser::parse_decl() {
    if (tokens_.check_keyword("const")) {
        return parse_const_decl();
    }
    return parse_var_decl();
}

std::unique_ptr<VarDecl> Parser::parse_const_decl() {
    const Token &const_token = tokens_.expect_keyword("const");
    const BasicType base_type = parse_basic_type();
    auto declaration = std::make_unique<VarDecl>(token_range(const_token), base_type, true);

    declaration->definitions.push_back(parse_var_def(true));
    while (tokens_.match_punct(",")) {
        declaration->definitions.push_back(parse_var_def(true));
    }
    const Token &semicolon = tokens_.expect_punct(";");
    declaration->range.end = token_range(semicolon).end;
    return declaration;
}

std::unique_ptr<VarDecl> Parser::parse_var_decl() {
    const SourceLoc begin = tokens_.current().loc;
    const BasicType base_type = parse_basic_type();
    auto declaration = std::make_unique<VarDecl>(SourceRange{begin, begin}, base_type, false);

    declaration->definitions.push_back(parse_var_def(false));
    while (tokens_.match_punct(",")) {
        declaration->definitions.push_back(parse_var_def(false));
    }
    const Token &semicolon = tokens_.expect_punct(";");
    declaration->range.end = token_range(semicolon).end;
    return declaration;
}

VarDef Parser::parse_var_def(const bool constant_context) {
    const Token &name_token = tokens_.expect(TokenKind::IDENT);
    VarDef definition{token_range(name_token), std::string(lexeme(file_, name_token)), {}, nullptr};

    while (tokens_.match_punct("[")) {
        definition.dimensions.push_back(parse_const_exp());
        const Token &close_bracket = tokens_.expect_punct("]");
        definition.range.end = token_range(close_bracket).end;
    }

    if (tokens_.match_punct("=")) {
        definition.initializer = parse_initializer(constant_context);
        definition.range.end = definition.initializer->range.end;
    }
    return definition;
}

std::unique_ptr<Initializer> Parser::parse_initializer(const bool constant_context) {
    if (!tokens_.check_punct("{")) {
        ExprPtr expression = constant_context ? parse_const_exp() : parse_exp();
        const SourceRange range = expression->range;
        return std::make_unique<Initializer>(range, std::move(expression));
    }

    const Token &open_brace = tokens_.expect_punct("{");
    const SourceLoc begin = open_brace.loc;
    std::vector<std::unique_ptr<Initializer>> elements;
    if (!tokens_.check_punct("}")) {
        elements.push_back(parse_initializer(constant_context));
        while (tokens_.match_punct(",")) {
            if (tokens_.check_punct("}")) {
                unsupported_here("initializer after ','");
            }
            elements.push_back(parse_initializer(constant_context));
        }
    }
    const Token &close_brace = tokens_.expect_punct("}");
    return std::make_unique<Initializer>(
        SourceRange{begin, token_range(close_brace).end}, std::move(elements));
}

BasicType Parser::parse_basic_type(const bool allow_void) {
    if (tokens_.match_keyword("int")) {
        return BasicType::Int;
    }
    if (tokens_.match_keyword("float")) {
        return BasicType::Float;
    }
    if (allow_void && tokens_.match_keyword("void")) {
        return BasicType::Void;
    }
    unsupported_here(allow_void ? "basic type ('int', 'float', or 'void')"
                           : "basic type ('int' or 'float')");
}

std::unique_ptr<FuncDef> Parser::parse_func_def() {
    const SourceLoc begin = tokens_.current().loc;
    const BasicType return_type = parse_basic_type(true);
    const Token &name_token = tokens_.expect(TokenKind::IDENT);
    auto function = std::make_unique<FuncDef>(
        SourceRange{begin, begin}, return_type, std::string(lexeme(file_, name_token)));

    tokens_.expect_punct("(");
    if (!tokens_.check_punct(")")) {
        function->parameters.push_back(parse_func_param());
        while (tokens_.match_punct(",")) {
            function->parameters.push_back(parse_func_param());
        }
    }
    tokens_.expect_punct(")");
    function->body = parse_block();
    function->range.end = function->body->range.end;
    return function;
}

std::unique_ptr<FuncParam> Parser::parse_func_param() {
    const SourceLoc begin = tokens_.current().loc;
    const BasicType base_type = parse_basic_type();
    const Token &name_token = tokens_.expect(TokenKind::IDENT);
    auto parameter = std::make_unique<FuncParam>(
        token_range(name_token), base_type, std::string(lexeme(file_, name_token)));
    parameter->range.begin = begin;

    if (tokens_.match_punct("[")) {
        parameter->is_array = true;
        const Token &close_bracket = tokens_.expect_punct("]");
        parameter->range.end = token_range(close_bracket).end;
        while (tokens_.match_punct("[")) {
            parameter->dimensions.push_back(parse_exp());
            const Token &dimension_close = tokens_.expect_punct("]");
            parameter->range.end = token_range(dimension_close).end;
        }
    }
    return parameter;
}

std::unique_ptr<BlockStmt> Parser::parse_block() {
    const Token &open_brace = tokens_.expect_punct("{");
    auto block = std::make_unique<BlockStmt>(token_range(open_brace));

    while (!tokens_.check_punct("}")) {
        if (tokens_.eof()) {
            tokens_.expect_punct("}");
        }
        block->items.push_back(parse_block_item());
    }

    const Token &close_brace = tokens_.expect_punct("}");
    block->range.end = token_range(close_brace).end;
    return block;
}

BlockItem Parser::parse_block_item() {
    if (tokens_.check_keyword("const") ||
        (is_basic_type_keyword(tokens_) && !tokens_.check_keyword("void"))) {
        return BlockItem{parse_decl()};
    }
    return BlockItem{parse_stmt()};
}

StmtPtr Parser::parse_stmt() {
    if (tokens_.check_punct("{")) {
        return parse_block();
    }
    if (tokens_.check_keyword("if")) {
        return parse_if_stmt();
    }
    if (tokens_.check_keyword("while")) {
        return parse_while_stmt();
    }
    if (tokens_.check_keyword("break")) {
        const Token &keyword = tokens_.expect_keyword("break");
        const Token &semicolon = tokens_.expect_punct(";");
        return std::make_unique<BreakStmt>(merge_range(token_range(keyword), token_range(semicolon)));
    }
    if (tokens_.check_keyword("continue")) {
        const Token &keyword = tokens_.expect_keyword("continue");
        const Token &semicolon = tokens_.expect_punct(";");
        return std::make_unique<ContinueStmt>(merge_range(token_range(keyword), token_range(semicolon)));
    }
    if (tokens_.check_keyword("return")) {
        return parse_return_stmt();
    }
    if (tokens_.check_punct(";")) {
        const Token &semicolon = tokens_.expect_punct(";");
        return std::make_unique<EmptyStmt>(token_range(semicolon));
    }
    if (is_assignment_stmt()) {
        return parse_assign_stmt();
    }

    ExprPtr expression = parse_exp();
    const Token &semicolon = tokens_.expect_punct(";");
    return std::make_unique<ExprStmt>(
        SourceRange{expression->range.begin, token_range(semicolon).end}, std::move(expression));
}

std::unique_ptr<IfStmt> Parser::parse_if_stmt() {
    const Token &if_token = tokens_.expect_keyword("if");
    tokens_.expect_punct("(");
    ExprPtr condition = parse_cond();
    tokens_.expect_punct(")");
    StmtPtr then_branch = parse_stmt();
    auto statement = std::make_unique<IfStmt>(
        SourceRange{if_token.loc, then_branch->range.end}, std::move(condition), std::move(then_branch));
    if (tokens_.match_keyword("else")) {
        statement->else_branch = parse_stmt();
        statement->range.end = statement->else_branch->range.end;
    }
    return statement;
}

std::unique_ptr<WhileStmt> Parser::parse_while_stmt() {
    const Token &while_token = tokens_.expect_keyword("while");
    tokens_.expect_punct("(");
    ExprPtr condition = parse_cond();
    tokens_.expect_punct(")");
    StmtPtr body = parse_stmt();
    return std::make_unique<WhileStmt>(
        SourceRange{while_token.loc, body->range.end}, std::move(condition), std::move(body));
}

std::unique_ptr<ReturnStmt> Parser::parse_return_stmt() {
    const Token &return_token = tokens_.expect_keyword("return");
    auto statement = std::make_unique<ReturnStmt>(token_range(return_token));
    if (!tokens_.check_punct(";")) {
        statement->value = parse_exp();
    }
    const Token &semicolon = tokens_.expect_punct(";");
    statement->range.end = token_range(semicolon).end;
    return statement;
}

std::unique_ptr<AssignStmt> Parser::parse_assign_stmt() {
    std::unique_ptr<LValueExpr> target = parse_lvalue_expr();
    tokens_.expect_punct("=");
    ExprPtr value = parse_exp();
    const Token &semicolon = tokens_.expect_punct(";");
    return std::make_unique<AssignStmt>(
        SourceRange{target->range.begin, token_range(semicolon).end}, std::move(target), std::move(value));
}

bool Parser::is_assignment_stmt() const {
    if (!tokens_.check(TokenKind::IDENT)) {
        return false;
    }

    size_t offset = 1;
    size_t bracket_depth = 0;
    while (true) {
        const Token &token = tokens_.peek(offset++);
        if (token.kind == TokenKind::TK_EOF) {
            return false;
        }
        if (token.kind == TokenKind::PUNCT) {
            const std::string_view text = lexeme(file_, token);
            if (text == "[") {
                ++bracket_depth;
                continue;
            }
            if (text == "]") {
                if (bracket_depth == 0) {
                    return false;
                }
                --bracket_depth;
                continue;
            }
            if (bracket_depth == 0) {
                return text == "=";
            }
        }
    }
}

ExprPtr Parser::parse_exp() {
    return parse_add_exp();
}

ExprPtr Parser::parse_cond() {
    return parse_lor_exp();
}

ExprPtr Parser::parse_lor_exp() {
    ExprPtr expression = parse_land_exp();
    while (tokens_.match_punct("||")) {
        expression = make_binary(BinaryOp::LogicalOr, std::move(expression), parse_land_exp());
    }
    return expression;
}

ExprPtr Parser::parse_land_exp() {
    ExprPtr expression = parse_eq_exp();
    while (tokens_.match_punct("&&")) {
        expression = make_binary(BinaryOp::LogicalAnd, std::move(expression), parse_eq_exp());
    }
    return expression;
}

ExprPtr Parser::parse_eq_exp() {
    ExprPtr expression = parse_rel_exp();
    while (tokens_.check_punct("==") || tokens_.check_punct("!=")) {
        const bool equals = tokens_.match_punct("==");
        if (!equals) {
            tokens_.expect_punct("!=");
        }
        expression = make_binary(equals ? BinaryOp::Eq : BinaryOp::Ne,
                                 std::move(expression), parse_rel_exp());
    }
    return expression;
}

ExprPtr Parser::parse_rel_exp() {
    ExprPtr expression = parse_add_exp(true);
    while (tokens_.check_punct("<") || tokens_.check_punct("<=") ||
           tokens_.check_punct(">") || tokens_.check_punct(">=")) {
        BinaryOp op = BinaryOp::Lt;
        if (tokens_.match_punct("<")) {
            op = BinaryOp::Lt;
        } else if (tokens_.match_punct("<=")) {
            op = BinaryOp::Le;
        } else if (tokens_.match_punct(">")) {
            op = BinaryOp::Gt;
        } else {
            tokens_.expect_punct(">=");
            op = BinaryOp::Ge;
        }
        expression = make_binary(op, std::move(expression), parse_add_exp(true));
    }
    return expression;
}

ExprPtr Parser::parse_add_exp(const bool allow_logical_not) {
    ExprPtr expression = parse_mul_exp(allow_logical_not);
    while (tokens_.check_punct("+") || tokens_.check_punct("-")) {
        BinaryOp op = BinaryOp::Add;
        if (!tokens_.match_punct("+")) {
            tokens_.expect_punct("-");
            op = BinaryOp::Sub;
        }
        expression = make_binary(op, std::move(expression), parse_mul_exp(allow_logical_not));
    }
    return expression;
}

ExprPtr Parser::parse_mul_exp(const bool allow_logical_not) {
    ExprPtr expression = parse_unary_exp(allow_logical_not);
    while (tokens_.check_punct("*") || tokens_.check_punct("/") || tokens_.check_punct("%")) {
        BinaryOp op = BinaryOp::Mul;
        if (tokens_.match_punct("*")) {
            op = BinaryOp::Mul;
        } else if (tokens_.match_punct("/")) {
            op = BinaryOp::Div;
        } else {
            tokens_.expect_punct("%");
            op = BinaryOp::Mod;
        }
        expression = make_binary(op, std::move(expression), parse_unary_exp(allow_logical_not));
    }
    return expression;
}

ExprPtr Parser::parse_unary_exp(const bool allow_logical_not) {
    if (tokens_.check_punct("+") || tokens_.check_punct("-") ||
        (allow_logical_not && tokens_.check_punct("!"))) {
        const Token &operator_token = tokens_.advance();
        const std::string_view text = lexeme(file_, operator_token);
        const UnaryOp op = text == "+" ? UnaryOp::Plus
                          : text == "-" ? UnaryOp::Minus
                                        : UnaryOp::LogicalNot;
        ExprPtr operand = parse_unary_exp(allow_logical_not);
        return std::make_unique<UnaryExpr>(
            SourceRange{operator_token.loc, operand->range.end}, op, std::move(operand));
    }
    if (tokens_.check(TokenKind::IDENT) &&
        tokens_.peek(1).kind == TokenKind::PUNCT && lexeme(file_, tokens_.peek(1)) == "(") {
        return parse_call_expr();
    }
    return parse_primary_exp(allow_logical_not);
}

ExprPtr Parser::parse_primary_exp(const bool allow_logical_not) {
    if (tokens_.check_punct("(")) {
        const Token &open_paren = tokens_.expect_punct("(");
        ExprPtr expression = allow_logical_not ? parse_cond() : parse_exp();
        const Token &close_paren = tokens_.expect_punct(")");
        expression->range = merge_range(token_range(open_paren), token_range(close_paren));
        return expression;
    }
    if (tokens_.check(TokenKind::INT_LITERAL)) {
        const Token &token = tokens_.advance();
        return std::make_unique<IntLiteral>(token_range(token), token.int_val);
    }
    if (tokens_.check(TokenKind::FLOAT_LITERAL)) {
        const Token &token = tokens_.advance();
        return std::make_unique<FloatLiteral>(token_range(token), token.float_val);
    }
    if (tokens_.check(TokenKind::IDENT)) {
        return parse_lvalue_expr();
    }
    unsupported_here("primary expression");
}

ExprPtr Parser::parse_const_exp() {
    return parse_add_exp();
}

std::unique_ptr<LValueExpr> Parser::parse_lvalue_expr() {
    const Token &name_token = tokens_.expect(TokenKind::IDENT);
    auto lvalue = std::make_unique<LValueExpr>(
        token_range(name_token), std::string(lexeme(file_, name_token)));
    while (tokens_.match_punct("[")) {
        lvalue->indices.push_back(parse_exp());
        const Token &close_bracket = tokens_.expect_punct("]");
        lvalue->range.end = token_range(close_bracket).end;
    }
    return lvalue;
}

std::unique_ptr<CallExpr> Parser::parse_call_expr() {
    const Token &name_token = tokens_.expect(TokenKind::IDENT);
    auto call = std::make_unique<CallExpr>(
        token_range(name_token), std::string(lexeme(file_, name_token)));
    tokens_.expect_punct("(");
    if (!tokens_.check_punct(")")) {
        call->arguments.push_back(parse_exp());
        while (tokens_.match_punct(",")) {
            call->arguments.push_back(parse_exp());
        }
    }
    const Token &close_paren = tokens_.expect_punct(")");
    call->range.end = token_range(close_paren).end;
    return call;
}

ExprPtr Parser::make_binary(const BinaryOp op, ExprPtr lhs, ExprPtr rhs) const {
    const SourceRange range = merge_range(lhs->range, rhs->range);
    return std::make_unique<BinaryExpr>(range, op, std::move(lhs), std::move(rhs));
}

[[noreturn]] void Parser::unsupported_here(const std::string_view expected) const {
    const Token &token = tokens_.current();
    std::string message = "expected ";
    message.append(expected.data(), expected.size());
    message += ", got ";
    if (token.kind == TokenKind::TK_EOF) {
        message += "end of file";
    } else {
        const std::string_view text = lexeme(file_, token);
        message += "'";
        message.append(text.data(), text.size());
        message += "'";
    }
    throw Error::CompileError{Error::ErrorCode::PARSE_UNEXPECTED_TOKEN, token.loc,
                              message};
}
