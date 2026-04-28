#include "lexer.h"

#include <cctype>
#include <ostream>

namespace lexer {

std::ostream& operator<<(std::ostream& out, const Token& token) {
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

namespace {

auto skip_spaces(const std::string& s, size_t& p) noexcept -> void {
    while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p]))) {
        ++p;
    }
}

auto skip_op(const std::string& s, size_t& p) noexcept -> void {
    while (p < s.size() && (s[p] == '+' || s[p] == '-')) {
        ++p;
    }
}

auto scan_text(const std::string& s, size_t& p) noexcept -> size_t {
    const size_t start = p;
    while (p < s.size() && s[p] != '|') {
        ++p;
    }
    return p - start;
}

auto scan(std::vector<Token>& tokens,
          const std::string& line,
          size_t& p,
          size_t line_no) noexcept -> void {
    skip_spaces(line, p);
    skip_op(line, p);

    while (true) {
        if (p >= line.size()) {
            tokens.push_back(Token{Kind::EndOfLine, p, 0, line_no});
            return;
        }

        if (line[p] == '|') {
            tokens.push_back(Token{Kind::Pipe, p, 1, line_no});
            ++p;
            skip_spaces(line, p);
        } else {
            const size_t start = p;
            const size_t length = scan_text(line, p);

            if (length == 0) {
                tokens.push_back(Token{Kind::Invalid, p, 1, line_no});
                ++p;
            } else {
                tokens.push_back(Token{Kind::Text, start, length, line_no});
            }
        }
    }
}

} // namespace

std::vector<Token> tokenize(const std::string& input, size_t line_no) {
    std::vector<Token> tokens;
    size_t position = 0;
    scan(tokens, input, position, line_no);
    return tokens;
}

} // namespace lexer