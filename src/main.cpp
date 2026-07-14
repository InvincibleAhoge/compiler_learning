#include "compiler/Sysy.h"
#include "compiler/ast_dump.h"
#include "compiler/error.h"
#include "compiler/parser.h"
#include "compiler/token.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

std::string open_file(const std::string &path) {
    std::ifstream out(path, std::ios::ate | std::ios::binary);
    if (!out.is_open())
        throw Error::CompileError{
            Error::ErrorCode::FILE_OPEN_FAILED,
            "cannot open file"
        };

    const std::streamsize size = out.tellg();
    out.seekg(0, std::ios::beg);
    std::string buffer(size, '\n');
    if (size > 0)
        out.read(buffer.data(), size);
    return buffer;
}

fs::path make_output_path(const std::string &source_path) {
    const fs::path output_dir = "output";
    fs::create_directories(output_dir);
    fs::path output_name = fs::path(source_path).filename();
    output_name.replace_extension(".tok");
    return output_dir / output_name;
}

int main(const int argc, char *argv[]) {
    File file;

    try {
        bool dump_ast_requested = false;
        const char *source_path = nullptr;
        if (argc == 2) {
            source_path = argv[1];
        } else if (argc == 3 && std::string_view{argv[1]} == "--dump-ast") {
            dump_ast_requested = true;
            source_path = argv[2];
        } else {
            throw Error::CompileError{
                Error::ErrorCode::FILE_OPEN_FAILED,
                "usage: Sysy [--dump-ast] <file.sy>"
            };
        }

        file.name = source_path;
        file.contents = open_file(file.name);

        auto tokens = tokenize(file);

        if (dump_ast_requested) {
            Parser parser(file, std::move(tokens));
            const auto unit = parser.parse();
            dump_ast(*unit, std::cout);
            return 0;
        }

#ifdef SYSY_PRINT_TOKENS
        const fs::path output_path = make_output_path(file.name);
        std::ofstream output(output_path);
        if (!output.is_open()) {
            throw Error::CompileError{
                Error::ErrorCode::OUTPUT_OPEN_FAILED,
                "cannot create output file"
            };
        }
        print_tokens(file, tokens, output);
#endif

        return 0;
    } catch (const Error::CompileError &err) {
        Error::print_error(file, err, std::cerr);
        return Error::error_code_value(err.code);
    } catch (const std::exception &err) {
        std::cerr << "[E9001] internal error: " << err.what() << '\n';
        return Error::error_code_value(Error::ErrorCode::InternalError);
    }
}
