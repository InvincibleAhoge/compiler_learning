#pragma once

#include <ostream>
#include <vector>
#include <string_view>
#include "Sysy.h"

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

// class TokenStream {
//     std::vector<Token> tokens_;
//     size_t index_ = 0;
//
// public:
//     explicit TokenStream(std::vector<Token> tokens)
//         : tokens_{std::move(tokens)} {
//     }
//
//     [[nodiscard]] const Token &peek(size_t offset = 0) const {
//         size_t pos = index_ + offset;
//         if (pos >= tokens_.size())
//             return tokens_.back(); // it should be EOF
//         return tokens_[pos];
//     }
//
//     [[nodiscard]] const Token &current() const { return peek(0); }
//
//     bool eof() const { return current().kind == TokenKind::TK_EOF; }
//
//     const Token &advance() {
//         const Token &tok = current();
//         if (!eof())
//             ++index_;
//         return tok;
//     }
//
//     bool check(TokenKind kind) const { return current().kind == kind; }
//
//     bool check_punct(std::string_view s) const {
//         return current().kind == TokenKind::PUNCT && current().lexeme == s;
//     }
//
//     bool check_keyword(std::string_view s) const {
//         return current().kind == TokenKind::KEYWORD && current().lexeme == s;
//     }
//
//     bool match_punct(std::string_view s) {
//         if (!check_punct(s))
//             return false;
//         advance();
//         return true;
//     }
//
//     bool match_keyword(std::string_view s) {
//         if (!check_keyword(s))
//             return false;
//         advance();
//         return true;
//     }
//
//     const Token &expect_punct(std::string_view s) {
//         if (!check_punct(s)) {
//             // throw ParserError{current().loc, "expected ..."};
//         }
//         return advance();
//     }
//
//     const Token &expect(TokenKind kind) {
//         if (!check(kind)) {
//             // throw ParserError{current().loc, "unexpected token"};
//         }
//         return advance();
//     }
//
//     size_t mark() const { return index_; }
//     void reset(size_t mark) { index_ = mark; }
// };
