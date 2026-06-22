#pragma once

#include "Sysy.h"
#include "error.h"

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

// Token
enum class TokenKind {
    IDENT, // identifiers
    PUNCT, // punctuators
    KEYWORD, // keywords
    INT_LITERAL, // integer
    FLOAT_LITERAL, // float
    STR, // string literal
    TK_EOF, // end of file
};

// Token type
struct Token {
    TokenKind kind;
    SourceLoc loc;
    size_t length = 0;

    sysy::i32 int_val = 0;
    sysy::f32 float_val = 0.0f;

    Token() = default;

    Token(const TokenKind kind, const size_t loc, const size_t length)
        : kind(kind),
          loc(loc),
          length(length) {
    }

    // 辅助访问函数
    [[nodiscard]]
    sysy::i32 get_i32() const { return int_val; }

    [[nodiscard]]
    sysy::f32 get_f32() const { return float_val; }
};

std::vector<Token> tokenize(File &file);

void print_tokens(const File &file, const std::vector<Token> &tokens, std::ostream &out);

std::string_view lexeme(const File &file, const Token &token);

class TokenStream {
    const File &file_;
    const std::vector<Token> &tokens_;
    size_t index_ = 0;

    [[nodiscard]]
    const Token &last_token() const {
        if (tokens_.empty())
            throw Error::CompileError{Error::ErrorCode::InternalError, "empty token stream"};
        return tokens_.back();
    }

    [[nodiscard]]
    std::string describe_current() const {
        const Token &token = current();
        if (token.kind == TokenKind::TK_EOF)
            return "end of file";

        const std::string_view text = lexeme(file_, token);
        std::string description = "'";
        description.append(text.data(), text.size());
        description += "'";
        return description;
    }

    [[noreturn]]
    void unexpected(std::string_view expected) const {
        const Token &token = current();
        std::string message = "expected ";
        message.append(expected.data(), expected.size());
        message += ", got ";
        message += describe_current();
        throw Error::CompileError{Error::ErrorCode::PARSE_UNEXPECTED_TOKEN, token.loc, message};
    }

public:
    TokenStream(const File &file, const std::vector<Token> &tokens)
        : file_{file},
          tokens_{tokens} {
    }

    [[nodiscard]]
    const File &file() const {
        return file_;
    }

    [[nodiscard]]
    const Token &peek(size_t offset = 0) const {
        const size_t pos = index_ + offset;
        if (pos >= tokens_.size())
            return last_token();
        return tokens_[pos];
    }

    [[nodiscard]]
    const Token &current() const {
        return peek();
    }

    [[nodiscard]]
    std::string_view current_lexeme() const {
        return lexeme(file_, current());
    }

    [[nodiscard]]
    bool eof() const {
        return current().kind == TokenKind::TK_EOF;
    }

    [[nodiscard]]
    bool check(TokenKind kind) const {
        return current().kind == kind;
    }

    [[nodiscard]]
    bool check_punct(std::string_view punct) const {
        return check(TokenKind::PUNCT) && lexeme(file_, current()) == punct;
    }

    [[nodiscard]]
    bool check_keyword(std::string_view keyword) const {
        return check(TokenKind::KEYWORD) && lexeme(file_, current()) == keyword;
    }

    [[nodiscard]]
    bool check_ident() const {
        return check(TokenKind::IDENT);
    }

    [[nodiscard]]
    bool check_number() const {
        return check(TokenKind::INT_LITERAL) || check(TokenKind::FLOAT_LITERAL);
    }

    [[nodiscard]]
    bool check_int_literal() const {
        return check(TokenKind::INT_LITERAL);
    }

    [[nodiscard]]
    bool check_float_literal() const {
        return check(TokenKind::FLOAT_LITERAL);
    }

    const Token &advance() {
        const Token &token = current();
        if (!eof())
            ++index_;
        return token;
    }

    bool match(TokenKind kind) {
        if (!check(kind))
            return false;
        advance();
        return true;
    }

    bool match_punct(std::string_view punct) {
        if (!check_punct(punct))
            return false;
        advance();
        return true;
    }

    bool match_keyword(std::string_view keyword) {
        if (!check_keyword(keyword))
            return false;
        advance();
        return true;
    }

    bool match_ident() {
        return match(TokenKind::IDENT);
    }

    bool match_number() {
        if (!check_number())
            return false;
        advance();
        return true;
    }

    const Token &expect(TokenKind kind, std::string_view expected) {
        if (!check(kind))
            unexpected(expected);
        return advance();
    }

    const Token &expect_punct(std::string_view punct) {
        if (!check_punct(punct)) {
            std::string expected = "'";
            expected.append(punct.data(), punct.size());
            expected += "'";
            unexpected(expected);
        }
        return advance();
    }

    const Token &expect_keyword(std::string_view keyword) {
        if (!check_keyword(keyword)) {
            std::string expected = "keyword '";
            expected.append(keyword.data(), keyword.size());
            expected += "'";
            unexpected(expected);
        }
        return advance();
    }

    const Token &expect_ident() {
        return expect(TokenKind::IDENT, "identifier");
    }

    const Token &expect_number() {
        if (!check_number())
            unexpected("number");
        return advance();
    }

    const Token &expect_int_literal() {
        return expect(TokenKind::INT_LITERAL, "integer literal");
    }

    const Token &expect_float_literal() {
        return expect(TokenKind::FLOAT_LITERAL, "floating literal");
    }

    [[nodiscard]]
    size_t mark() const {
        return index_;
    }

    void reset(size_t mark) {
        index_ = mark;
    }
};
