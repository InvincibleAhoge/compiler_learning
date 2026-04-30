#pragma once

#include <cstdint>
#include <string>

namespace sysy {
  using i32 = std::int32_t;
  using u32 = std::uint32_t;
  using f32 = float;
}


struct File {
  std::string name;       // 文件名
  std::string contents;   // 完整源码内容
};

