#pragma once

#include "Sysy.h"
#include "token.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// A non-owning cursor over lexer output.  Parser owns the token vector so
// references returned by this class remain stable for the parser lifetime.
class TokenStream {
public:
    TokenStream(const File &file, const std::vector<Token> &tokens);

    [[nodiscard]] const Token &peek(size_t offset = 0) const;
    [[nodiscard]] const Token &current() const;
    [[nodiscard]] bool eof() const;
    const Token &advance();

    [[nodiscard]] bool check(TokenKind kind) const;
    [[nodiscard]] bool check_punct(std::string_view text) const;
    [[nodiscard]] bool check_keyword(std::string_view text) const;

    bool match(TokenKind kind);
    bool match_punct(std::string_view text);
    bool match_keyword(std::string_view text);

    const Token &expect(TokenKind kind);
    const Token &expect_punct(std::string_view text);
    const Token &expect_keyword(std::string_view text);

private:
    [[noreturn]] void unexpected(std::string_view expected) const;
    [[nodiscard]] std::string describe(const Token &token) const;

    const File &file_;
    const std::vector<Token> &tokens_;
    size_t index_ = 0;
};
