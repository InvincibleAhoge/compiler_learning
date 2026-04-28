//
// Created by LeoXavier on 2026/4/26.
//
#include "reader.h"
#include <fstream>
#include <iterator>
#include <stdexcept>

using std::ifstream;
using std::string;
using std::vector;
namespace fs = std::filesystem;

namespace reader {
    string read_file(const string &file_name) {
        ifstream file(file_name);
        if (!file)
            throw std::runtime_error("Cannot open file");
        return string{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };
    }
}
