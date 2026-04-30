#include "Sysy.h"
#include "token.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>

std::string open_file1(const std::string &path) {
  std::ifstream out(path, std::ios::ate | std::ios::binary);
  if (!out.is_open())
    throw std::runtime_error{"cannot open file"};

  std::streamsize size = out.tellg();
  out.seekg(0, std::ios::beg);
  std::string buffer(size, '\n');
  if (size > 0)
    out.read(buffer.data(), size);
  return buffer;
}

std::string open_file(const std::string &path) {
  std::ifstream out(path, std::ios::binary);
  if (!out.is_open())
    throw std::runtime_error{"cannot open file"};
  std::stringstream buffer;
  buffer << out.rdbuf();
  return buffer.str();
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "usage: sysyc <file.sy>\n";
    return 1;
  }

  File file;
  file.name = argv[1];
  std::cout << "正在编译文件：" << file.name << std::endl;

#ifdef USE_FAST_IO
  std::cout << ">> 启用模式: 高性能读取 (Method 1)" << std::endl;
  file.contents = open_file1(file.name);
#else
  std::cout << ">> 启用模式: 标准读取 (Method 2)" << std::endl;
  file.contents = open_file(file.name);
#endif

  // 暂时的变量
  auto tokens = tokenize(file);
  print_tokens(tokens);
  TokenStream ts{std::move(tokens)};

  return 0;
}