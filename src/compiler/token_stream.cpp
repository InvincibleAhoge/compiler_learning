#include "token_stream.h"

#include "error.h"

#include <string>

namespace {

std::string_view token_kind_description(const TokenKind kind) {
    switch (kind) {
    case TokenKind::IDENT:
        return "identifier";
    case TokenKind::PUNCT:
        return "punctuator";
    case TokenKind::KEYWORD:
        return "keyword";
    case TokenKind::INT_LITERAL:
        return "integer literal";
    case TokenKind::FLOAT_LITERAL:
        return "floating literal";
    case TokenKind::STR:
        return "string literal";
    case TokenKind::TK_EOF:
        return "end of file";
    }
    return "token";
}

} // namespace

TokenStream::TokenStream(const File &file, const std::vector<Token> &tokens)
    : file_(file), tokens_(tokens) {
    if (tokens_.empty() || tokens_.back().kind != TokenKind::TK_EOF) {
        throw Error::CompileError{Error::ErrorCode::InternalError,
                                  "token stream must end with an EOF token"};
    }
}

const Token &TokenStream::peek(const size_t offset) const {
    const size_t remaining = tokens_.size() - index_;
    if (offset >= remaining) {
        return tokens_.back();
    }
    return tokens_[index_ + offset];
}

const Token &TokenStream::current() const {
    return peek();
}

bool TokenStream::eof() const {
    return current().kind == TokenKind::TK_EOF;
}

const Token &TokenStream::advance() {
    const Token &token = current();
    if (!eof()) {
        ++index_;
    }
    return token;
}

bool TokenStream::check(const TokenKind kind) const {
    return current().kind == kind;
}

bool TokenStream::check_punct(const std::string_view text) const {
    return check(TokenKind::PUNCT) && lexeme(file_, current()) == text;
}

bool TokenStream::check_keyword(const std::string_view text) const {
    return check(TokenKind::KEYWORD) && lexeme(file_, current()) == text;
}

bool TokenStream::match(const TokenKind kind) {
    if (!check(kind)) {
        return false;
    }
    advance();
    return true;
}

bool TokenStream::match_punct(const std::string_view text) {
    if (!check_punct(text)) {
        return false;
    }
    advance();
    return true;
}

bool TokenStream::match_keyword(const std::string_view text) {
    if (!check_keyword(text)) {
        return false;
    }
    advance();
    return true;
}

const Token &TokenStream::expect(const TokenKind kind) {
    if (!check(kind)) {
        unexpected(token_kind_description(kind));
    }
    return advance();
}

const Token &TokenStream::expect_punct(const std::string_view text) {
    if (!check_punct(text)) {
        std::string expected = "punctuator '";
        expected.append(text.data(), text.size());
        expected += "'";
        unexpected(expected);
    }
    return advance();
}

const Token &TokenStream::expect_keyword(const std::string_view text) {
    if (!check_keyword(text)) {
        std::string expected = "keyword '";
        expected.append(text.data(), text.size());
        expected += "'";
        unexpected(expected);
    }
    return advance();
}

std::string TokenStream::describe(const Token &token) const {
    if (token.kind == TokenKind::TK_EOF) {
        return "end of file";
    }

    const std::string_view text = lexeme(file_, token);
    std::string result = "'";
    result.append(text.data(), text.size());
    result += "'";
    return result;
}

[[noreturn]] void TokenStream::unexpected(const std::string_view expected) const {
    std::string message = "expected ";
    message.append(expected.data(), expected.size());
    message += ", got ";
    message += describe(current());
    throw Error::CompileError{Error::ErrorCode::PARSE_UNEXPECTED_TOKEN,
                              current().loc, message};
}
