# Repository Guidelines

## 项目结构与模块组织
本仓库现为基于 CMake 的 C++ 项目，用于将 HPU 汇编转换为指令编码。公共头文件放在 `include/hpu/`，对外暴露解析、编码和组装接口；实现放在 `src/`，当前核心模块包括 `parser.cpp`、`encoder.cpp`、`assembler.cpp` 和 `instruction.cpp`。命令行入口是 `src/main.cpp`。测试代码放在 `tests/`，每个 `*_test.cpp` 都会单独生成一个测试可执行文件到 `build/`。`build/` 仅存放生成产物，不要手工编辑。

## 构建、测试与开发命令
推荐使用以下命令：

- `cmake -S . -B build`：生成构建系统。
- `cmake --build build -j4`：编译库、CLI 和测试程序。
- `ctest --test-dir build --output-on-failure`：运行全部测试。
- `cmake --build build --clean-first -j4`：强制干净重编。
- `./build/hpu_encode_cli asm.txt`：将汇编文件转换为 32 位编码。

如果只想检查单个测试，也可以直接运行 `./build/parser_test` 或 `./build/custom1_encoder_test`。

## 代码风格与命名约定
使用 4 空格缩进，左花括号与语句同行。类型、枚举和结构体命名保持清晰直接；函数与局部变量使用 `snake_case`，常量使用 `kCamelCase` 或 `UPPER_SNAKE_CASE`，遵循现有代码即可。解析器和编码器优先保持显式逻辑，不要为了“通用化”牺牲可读性。新增字段时，优先在头文件中补充清晰的数据模型，再落到编码实现。

## 测试指南
测试框架使用 GoogleTest。每个测试文件聚焦一类行为，例如解析、`custom0` 编码、`custom1` 编码或整段内联汇编组装。新增指令或语法时，至少补充：一个合法样例、一个非法样例，以及一个明确校验编码值的断言。若语法涉及十进制/十六进制、可选后缀或注释过滤，应覆盖这些边界情况。

## 提交与 Pull Request 规范
提交信息建议使用简洁的祈使句，并带上范围，例如 `add dma irq parsing`、`refine custom0 mem encoding`。PR 说明至少包含：修改目标、受影响的指令或模块、构建命令、测试结果；如果变更了汇编语法或编码位定义，请给出示例输入与期望输出。
