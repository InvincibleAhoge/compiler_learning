//
// Created by LeoXavier on 2026/5/21.
//

#ifndef MAIN_CPP_ERROR_HANDLER_H
#define MAIN_CPP_ERROR_HANDLER_H
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>

#include "Sysy.h"

namespace Error {
    enum class ErrorCode {
        OK = 0,

        FILE_OPEN_FAILED = 101,
        OUTPUT_OPEN_FAILED = 102,

        LEX_INVALID_TOKEN = 201,
        LEX_UNCLOSED_BLOCK_COMMENT = 202,
        LEX_INVALID_INTEGER = 203,
        LEX_INVALID_FLOAT = 204,
        LEX_INT_OVERFLOW = 205,
        LEX_FLOAT_OVERFLOW = 206,

        PARSE_UNEXPECTED_TOKEN = 301,

        InternalError = 901,
    };

    class CompileError : public std::runtime_error {
    public:
        ErrorCode code;
        std::optional<SourceLoc> loc;

        CompileError(const ErrorCode code, const std::string &msg) : std::runtime_error(msg), code(code) {
        }

        CompileError(const ErrorCode code, SourceLoc loc, const std::string &msg) : std::runtime_error(msg),
            code(code),
            loc(loc) {
        }
    };

    static int error_code_value(ErrorCode code) {
        return static_cast<int>(code);
    }

    void print_error(const File &file, const Error::CompileError &err, std::ostream &out);
}

#endif //MAIN_CPP_ERROR_HANDLER_H
