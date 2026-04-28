#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace lexer {
    enum class Kind {
        Text,
        Pipe,
        EndOfLine,
        Invalid
    };

    struct Token {
        Kind kind;
        size_t start;
        size_t length;
        size_t line_no;
    };

    std::ostream& operator<<(std::ostream& out, const Token &token);

    std::vector<Token> tokenize(const std::string &input, size_t line_no);
}