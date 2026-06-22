#include "Sysy.h"
#include "error.h"
#include "token.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

void canonicalize_newline(std::string &str) {
  size_t write = 0;
  for (size_t read = 0; read < str.length(); read++) {
    if (str[read] == '\r') {
      if (read + 1 < str.length() && str[read + 1] == '\n')
        read++;

      str[write] = '\n';
      write++;
    } else {
      str[write] = str[read];
      write++;
    }
  }
  str.resize(write);
  str.push_back('\0');
}

bool is_digit(const char c) { return c >= '0' && c <= '9'; }

bool is_alpha(const char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_alpha_and_num(const char c) { return is_alpha(c) || is_digit(c); }

bool is_space(const char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' ||
         c == '\v';
}

bool startswith(const char *p, const char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

bool startswith(const std::string_view s, const std::string_view prefix) {
  return s.size() >= prefix.length() && s.substr(0, prefix.length()) == prefix;
}

bool is_keyword(const char *p, const size_t len) {
  static const char *keywords[] = {"break", "const", "continue", "else",
                                   "float", "if",    "int",      "return",
                                   "void",  "while"};
  for (const auto kw : keywords)
    if (strlen(kw) == len && strncmp(p, kw, len) == 0)
      return true;
  return false;
}

size_t read_punct(const char *p) {
  switch (*p) {
  case '=':
  case '!':
  case '<':
  case '>':
    return p[1] == '=' ? 2 : 1;
  case '&':
    return p[1] == '&' ? 2 : 0;
  case '|':
    return p[1] == '|' ? 2 : 0;
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '(':
  case ')':
  case '[':
  case ']':
  case '{':
  case '}':
  case ',':
  case ';':
    return 1;
  default:
    return 0;
  }
}

bool is_float_literal(const std::string_view s) {
  if (startswith(s, "0x") || startswith(s, "0X"))
    return s.find('p') != std::string::npos || s.find('P') != std::string::npos;

  if (s.find('.') != std::string::npos)
    return true;

  return s.find('e') != std::string::npos || s.find('E') != std::string::npos;
}

// constexpr bool is_overflow(long long num) {
//     constexpr long long MAX_NUMERIC_LITERAL = 2414483648;
//     return num > MAX_NUMERIC_LITERAL;
// }

std::vector<Token> tokenize(File &file) {
  canonicalize_newline(file.contents);
  build_line_offsets(file);

  const char *p = file.contents.data();
  std::vector<Token> tokens;

  while (*p) {
    // Skip line comment
    if (startswith(p, "//")) {
      p += 2;
      while (*p && *p != '\n')
        p++;
      continue;
    }
    // Skip block comment
    if (startswith(p, "/*")) {
      const size_t start = p - file.contents.data();
      const size_t q = file.contents.find("*/", start + 2);
      if (q == std::string::npos)
        throw Error::CompileError(Error::ErrorCode::LEX_UNCLOSED_BLOCK_COMMENT,
                                  SourceLoc{start}, "unclosed block comment");
      p = &file.contents[q + 2];
      continue;
    }

    // Skip whitespace characters
    if (is_space(*p)) {
      p++;
      continue;
    }

    // Numeric literal
    if (is_digit(*p) || (*p == '.' && is_digit(*(p + 1)))) {
      const char *start = p++;
      while (true) {
        if (*p && *(p + 1) && strchr("eEpP", *p) && strchr("+-", *(p + 1)))
          p += 2;
        else if (is_alpha_and_num(*p) || *p == '.')
          p++;
        else
          break;
      }

      const auto len = static_cast<size_t>(p - start);
      const auto offset = static_cast<size_t>(start - file.contents.data());
      auto &token = tokens.emplace_back(TokenKind::INT_LITERAL, offset, len);

      std::string_view sv(start, len);
      std::string num_str(sv);
      const bool is_float = is_float_literal(sv);
      char *end = nullptr;

      if (is_float) {
        token.kind = TokenKind::FLOAT_LITERAL;
        token.float_val = std::strtof(num_str.c_str(), &end);

        if (*end != '\0')
          throw Error::CompileError{Error::ErrorCode::LEX_INVALID_FLOAT,
                                    SourceLoc{offset},
                                    "invalid floating literal"};
      } else {
        const auto val = std::strtol(num_str.c_str(), &end, 0);

        if (*end != '\0')
          throw Error::CompileError{Error::ErrorCode::LEX_INVALID_INTEGER,
                                    SourceLoc{offset}, "invalid integer"};
        token.int_val = static_cast<sysy::i32>(val);
      }
      continue;
    }

    // String literal
    // Char literal
    // Sysy2022 貌似不支持 string 和 char，这里就先偷懒不做

    // identifier or keyword
    if (is_alpha(*p) || *p == '_') {
      const char *start = p++;
      while (*p && (is_alpha(*p) || *p == '_' || is_digit(*p)))
        p++;
      const auto len = static_cast<size_t>(p - start);
      const TokenKind kind =
          is_keyword(start, len) ? TokenKind::KEYWORD : TokenKind::IDENT;
      tokens.emplace_back(kind, start - file.contents.data(), len);
      continue;
    }

    // Punctuators
    if (size_t punct_len = read_punct(p)) {
      tokens.emplace_back(TokenKind::PUNCT, p - file.contents.data(),
                          punct_len);
      p += punct_len;
      continue;
    }

    throw Error::CompileError{
        Error::ErrorCode::LEX_INVALID_TOKEN,
        SourceLoc{static_cast<size_t>(p - file.contents.data())},
        "invalid token"};
  }
  tokens.emplace_back(TokenKind::TK_EOF, p - file.contents.data(), 0);
  return tokens;
}

static const char *token_kind_name(const TokenKind kind) {
  switch (kind) {
  case TokenKind::IDENT:
    return "identifier";
  case TokenKind::PUNCT:
    return "punctuator";
  case TokenKind::KEYWORD:
    return "keyword";
  case TokenKind::INT_LITERAL:
    return "int_literal";
  case TokenKind::FLOAT_LITERAL:
    return "float_literal";
  case TokenKind::STR:
    return "string";
  case TokenKind::TK_EOF:
    return "eof";
  }
  return "unknown";
}

void print_tokens(const File &file, const std::vector<Token> &tokens,
                  std::ostream &out) {
  // Compute max line:col width for alignment
  size_t max_pos_width = 0;
  for (const auto &token : tokens) {
    const LineCol pos = get_line_col(file, token.loc);
    const auto width =
        std::to_string(pos.line).size() + 1 + std::to_string(pos.column).size();
    if (width > max_pos_width)
      max_pos_width = width;
  }

  // Column widths
  static constexpr size_t kind_width = 14;
  const size_t pos_width = max_pos_width < 5 ? 5 : max_pos_width;

  // Header
  std::string str;
  str.append(pos_width - 3, ' ');
  str += "pos  kind          lexeme\n";
  str.append(pos_width - 3, ' ');
  str += "---  ----          ------\n";

  for (const auto &token : tokens) {
    const LineCol pos = get_line_col(file, token.loc);

    // Right-aligned line:col
    const auto pos_str =
        std::to_string(pos.line) + ":" + std::to_string(pos.column);
    str.append(pos_width - pos_str.size(), ' ');
    str += pos_str;
    str += "  ";

    // Token kind (left-aligned)
    const char *kind_name = token_kind_name(token.kind);
    str += kind_name;
    const auto kind_len = strlen(kind_name);
    if (kind_len < kind_width)
      str.append(kind_width - kind_len, ' ');

    // Lexeme
    if (token.kind != TokenKind::TK_EOF) {
      std::string_view lex(file.contents.data() + token.loc.offset,
                           token.length);
      str += '\'';
      str += lex;
      str += '\'';
    }

    str += '\n';
  }
  out << str << '\n';
}

std::string_view lexeme(const File &file, const Token &token) {
  return {file.contents.data() + token.loc.offset, token.length};
}
