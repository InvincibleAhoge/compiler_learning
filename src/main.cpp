#include "Sysy.h"
#include "token.h"
#include "error.h"

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
        if (argc < 2) {
            throw Error::CompileError{
                Error::ErrorCode::FILE_OPEN_FAILED,
                "usage: Sysy <file.sy>"
            };
        }

        file.name = argv[1];
        file.contents = open_file(file.name);

        const auto tokens = tokenize(file);

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
