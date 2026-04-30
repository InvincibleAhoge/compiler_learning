我已经把 lexer 的基础接口定好了，实现时主要只需关心一个函数:
``` cpp
    std::vector<Token> tokenize(const File &file);
```
File 里有两个字段：
``` c++
    struct File {
        std::string name;       // 文件名
        std::string contents;   // 完整源码内容
    };
```
lexer 要做的事：扫描 `file.contents`, 生成一组 `Token` ，最后必须追加一个 `TK_EOF`。

每个 Token 大概包含:
```cpp
    struct SourceLocation {
        sysy::u32 line;
        sysy::u32 column;
        size_t offset;  // 从源文件开头的偏移量
    };
    using Loc = SourceLocation;

    struct Token {
        TokenKind kind;
        std::string_view lexeme;
        sysy::i32 int_val;
        sysy::f32 float_val;
        Loc loc;
    }
```
实现要求：
1. 跳过空白字符和注释
2. 识别关键字，例如：int, float, void, if, else, while, return, continue, break, const ......
3. 识别标识符，类型为 `TokenKind::IDENT`
4. 识别字面量，类型分别为 `INT_LITERAL` 和 `FLOAT_LITERAL`
5. 识别运算符和分隔符，统一用 `TokenKind::PUNCT`，例如 + - * / % = == != < > ; , ......
6. 每个 token 的 lexeme 用 `std::string_view` 指向 `file.contents` 中的原始片段，不需要新建临时字符串
7. 每个 token 须记录 line / column / offset

运行方式：在 CMakeLists.txt 同级目录下，用cmake编译源代码，再 ./build/Sysy test/sys/accessible/example.sy

**注意：上传的文件还没有实现 `tokenize()` 和 `print_tokens()` ，务必在实现后编译，也可以暂时注释掉 `print_tokens()` ，换成你自己的方式。**

*其实应该是sysy目录，上传时忘记改了*