# NTT Case

本样例用于交付一份完整的 `NTT` 指令文件和对象级输入/输出约束。

目录内容：

- `ntt.asm`：N=64 的 stage 级 NTT 指令
- `input.txt`：对象槽位分配和输入多项式
- `expected.txt`：期望输出槽位和校验规则

约束说明：

- `p0` 为输入多项式对象
- `p1` 为 ping-pong 临时对象
- `p2` 为模上下文对象
- `p3` 为 shuffle 配置对象

当前文档没有固定公开 twiddle 常量和模上下文内部存储格式，因此这里的 `expected.txt` 采用“接口完整 + 校验规则明确”的方式交付，而不伪造数值黄金值。

生成编码：

```bash
./build/hpu_encode_cli cases/ntt/ntt.asm
```
