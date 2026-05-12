#pragma once

#include <string>
#include <vector>

namespace sysy {
    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using f32 = float;
}

struct File {
    std::string name; // 文件名
    std::string contents; // 完整源码内容

    // 每一行起始字符的 offset，0-based
    std::vector<size_t> line_offsets;
};

void build_line_offsets(File &file);

struct SourceLoc {
    size_t offset = 0; // 从源头算起
    SourceLoc() = default;

    explicit SourceLoc(const size_t offset) : offset(offset) {
    }
};

// struct SourceRange {
//     SourceLoc begin;
//     size_t length = 0;
// };

struct LineCol {
    size_t line; // 1-based
    size_t column; // 1-based
};

LineCol get_line_col(const File &file, SourceLoc loc);