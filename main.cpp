#include <iostream>
#include <string>
#include <sstream>

#include "reader.h"

#ifdef LEXER_TEST
#include "lexer.h"
void lexer_test();
#endif

#ifdef PARSER_TEST
#include "parser.h"
void parser_test();
#endif

using std::string;
using std::cout;
using std::getline;

void test();

int main() {
    test();
    return 0;
}

void test() {
#ifdef PARSER_TEST
    parser_test();
#endif

#ifdef LEXER_TEST
    lexer_test();
#endif
}

#ifdef LEXER_TEST

void lexer_test() {
    const string test_path{"../resource/test.txt"};
    const string content{reader::read_file(test_path)};

    std::istringstream ss(content);

    string line;
    size_t line_no = 0;

    while (getline(ss, line)) {
        ++line_no;

        if (line.empty()) 
            continue;

        const auto tokens = lexer::tokenize(line, line_no);

        for (const auto& token : tokens) 
            cout << line_no << ": " << token << '\n';
    }
}

#endif