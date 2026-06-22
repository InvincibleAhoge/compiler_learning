//
// Created by LeoXavier on 2026/5/21.
//

#include "error.h"

namespace Error {
    void print_error(const File &file, const CompileError &err, std::ostream &out) {
        out << "[E" << error_code_value(err.code) << "] ";

        if (!file.name.empty())
            out << file.name;

        if (err.loc.has_value()) {
            const LineCol pos = get_line_col(file, *err.loc);
            out << ":" << pos.line << ":" << pos.column;
        }
        out << ": " << err.what() << '\n';
    }
}
