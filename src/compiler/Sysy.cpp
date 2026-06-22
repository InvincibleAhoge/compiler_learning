//
// Created by LeoXavier on 2026/5/11.
//

#include "Sysy.h"
#include <algorithm>
#include <iterator>

void build_line_offsets(File &file) {
    file.line_offsets.clear();
    file.line_offsets.push_back(0);

    for (size_t i = 0; i < file.contents.size(); ++i)
        if (file.contents[i] == '\n')
            file.line_offsets.push_back(i + 1);
}

LineCol get_line_col(const File &file, const SourceLoc loc) {
    const auto it = std::upper_bound(
        file.line_offsets.begin(),
        file.line_offsets.end(),
        loc.offset
    );
    const auto line_index = static_cast<size_t>(
        std::distance(file.line_offsets.begin(), it) - 1
    );
    const size_t line_start = file.line_offsets[line_index];

    return {line_index + 1, loc.offset - line_start + 1};
}
