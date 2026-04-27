//
// Created by LeoXavier on 2026/4/13.
//
module;
#include <string>
#include <vector>
#include <sstream>
export module lexer;

using std::string;
using ul = size_t;

namespace lexer {
    export enum class Kind { Text, Pipe, EndOfLine, Invalid };

    export struct Token {
        Kind kind;
        ul start;
        ul length;
        ul line_no;
    };

    export inline std::ostream &operator<<(std::ostream &out, const Token &token) {
        out << "Token("
                << "kind=" << [&] {
                    switch (token.kind) {
                        case Kind::Text: return "text";
                        case Kind::Pipe: return "pipe";
                        case Kind::EndOfLine: return "end";
                        default: return "invalid";
                    }
                }()
                << ", start=" << token.start
                << ", length=" << token.length
                << ", line=" << token.line_no
                << ")";
        return out;
    }

    inline auto skip_spaces(const string &s, ul &p) noexcept -> void {
        while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p]))) ++p;
    }

    inline auto skip_op(const string &s, ul &p) noexcept -> void {
        while (p < s.size() && (s[p] == '+' || s[p] == '-')) p++;
    }

    inline auto scan_text(const string &s, ul &p) noexcept -> ul {
        const ul start = p;
        while (p < s.size() && s[p] != '|')
            ++p;
        return p - start;
    }

    inline auto scan(std::vector<Token> &tokens, const string &line, ul &p, const ul line_no) noexcept -> void {
        skip_spaces(line, p);
        skip_op(line, p);
        while (true) {
            if (p >= line.size()) {
                tokens.push_back(Token{Kind::EndOfLine, p, 0, line_no});
                return;
            }

            if (line[p] == '|') {
                tokens.push_back(Token{Kind::Pipe, p, 1, line_no});
                p++;
                skip_spaces(line, p);
            } else {
                const ul start = p;
                if (const ul length = scan_text(line, p); length == 0) {
                    tokens.push_back(Token{Kind::Invalid, p, 1, line_no});
                    p++;
                } else
                    tokens.push_back(Token{Kind::Text, start, length, line_no});
            }
        }
    }

    export auto tokenize(const string &input, const ul line_no) -> std::vector<Token> {
        std::vector<Token> tokens;
        ul position = 0;
        scan(tokens, input, position, line_no);
        return tokens;
    }
}
