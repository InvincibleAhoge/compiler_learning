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

SourceRange token_range(const Token &token);
