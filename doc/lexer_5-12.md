# SysY Lexer

本项目当前实现 SysY 源文件的词法分析阶段：读取 `.sy` 文件，规范化换行，切分 token，并可按编译期开关打印 token 列表。

## 当前状态

已实现：

- 源文件读取
- 换行规范化
- 行列号记录
- `//` 行注释和 `/* ... */` 块注释跳过
- 标识符识别
- 关键字识别
- 整数和浮点数字面量识别
- 常用运算符和分隔符识别
- 可选 token 打印

当前只覆盖词法分析阶段，尚未实现语法分析、语义分析、IR 生成或目标代码生成。

已知限制：

- 尚未实现字符串或格式字符串 token。
- 整数字面量的 32 位范围检查仍需补充。
- 浮点字面量规则仍需要继续按 SysY2022/C 规则收紧。

## 环境要求

- CMake 3.28 或更新版本
- 支持 C++17 的 C++ 编译器

Windows、Ubuntu、macOS 均可使用 CMake 构建。

## 快速构建

项目提供两份跨平台 CMake 脚本，Windows、Ubuntu、macOS 均使用同样命令运行。

### 高速读取并打印 tokens

```bash
cmake -P scripts/build-fast-tokens.cmake
```

该脚本会配置：

```text
USE_FAST_IO=ON
SYSY_PRINT_TOKENS=ON
```

构建目录：

```text
build-fast-tokens
```

### 高速读取但不打印 tokens

```bash
cmake -P scripts/build-fast.cmake
```

该脚本会配置：

```text
USE_FAST_IO=ON
SYSY_PRINT_TOKENS=OFF
```

构建目录：

```text
build-fast
```

## 手动 CMake 构建

也可以不用脚本，直接使用 CMake 命令。

### 默认构建

```bash
cmake -S . -B build
cmake --build build
```

### 高速读取

```bash
cmake -S . -B build-fast -DUSE_FAST_IO=ON -DSYSY_PRINT_TOKENS=OFF
cmake --build build-fast
```

### 高速读取并打印 tokens

```bash
cmake -S . -B build-fast-tokens -DUSE_FAST_IO=ON -DSYSY_PRINT_TOKENS=ON
cmake --build build-fast-tokens
```

## 运行

程序接收一个 SysY 源文件路径：

```bash
Sysy <file.sy>
```

macOS / Ubuntu 使用单配置生成器时，示例：

```bash
./build-fast/Sysy test/accessible/fib.sy
./build-fast-tokens/Sysy test/accessible/fib.sy
```

Windows 使用 Visual Studio 等多配置生成器时，产物通常位于配置子目录下，例如：

```powershell
.\build-fast\Debug\Sysy.exe test\accessible\fib.sy
.\build-fast-tokens\Debug\Sysy.exe test\accessible\fib.sy
```

如果使用 Ninja、Makefiles 或其他单配置生成器，Windows 产物也可能直接位于构建目录下：

```powershell
.\build-fast\Sysy.exe test\accessible\fib.sy
```

## 输出说明

未开启 `SYSY_PRINT_TOKENS` 时，只显示文件读取模式，例如：

```text
正在编译文件：test/accessible/fib.sy
>> 启用模式: 高性能读取 (Method 1)
```

开启 `SYSY_PRINT_TOKENS` 后，会额外打印 token 表：

```text
pos  kind          lexeme
---  ----          ------
1:1  keyword       'int'
1:5  identifier    'fib'
...
```

## CMake 选项

| 选项 | 默认值 | 说明 |
| --- | --- | --- |
| `USE_FAST_IO` | `OFF` | 使用一次分配、一次读取的文件读取方式 |
| `SYSY_PRINT_TOKENS` | `OFF` | 编译时开启 token 打印 |

## CLion 使用

CLion 默认会使用自己的 CMake Profile 和构建目录，通常是：

```text
cmake-build-debug
```

如果希望 CLion 内部构建也启用高速读取，在 CMake Profile 的 CMake options 中添加：

```text
-DUSE_FAST_IO=ON -DSYSY_PRINT_TOKENS=OFF
```

如果希望 CLion 内部构建打印 tokens：

```text
-DUSE_FAST_IO=ON -DSYSY_PRINT_TOKENS=ON
```

脚本构建目录和 CLion 默认构建目录彼此独立，互不影响。

## 清理构建目录

可以直接删除对应构建目录：

```bash
rm -rf build build-fast build-fast-tokens
```

Windows PowerShell：

```powershell
Remove-Item -Recurse -Force build, build-fast, build-fast-tokens
```
