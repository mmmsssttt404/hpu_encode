# Cases

本目录用于交付“指令 + 输入 + 期望结果”的成套样例，便于前端、译码器、RTL 和软件联调统一对齐。

每组样例至少包含：

- `*.asm`：要送入 `hpu_encode_cli` 的汇编文件
- `input.txt`：对象槽位输入、上下文对象和执行前约束
- `expected.txt`：期望输出对象、黄金值或校验规则
- `README.md`：该样例的用途和检查方法

当前包含：

- `ntt/`
- `intt/`
- `mm/`
- `bconv/`

生成编码文件的方法：

```bash
./build/hpu_encode_cli cases/ntt/ntt.asm
./build/hpu_encode_cli cases/intt/intt.asm
./build/hpu_encode_cli cases/mm/mm.asm
./build/hpu_encode_cli cases/bconv/bconv.asm
```

若需要逐行查看字段分段：

```bash
./build/asm_file_dump cases/mm/mm.asm
```
