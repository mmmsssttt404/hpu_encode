# 样例任务提交说明

本次交付内容面向 “PE 输入数据以及结果” 任务，目标是给出可直接用于译码、编码、联调和结果核对的成套样例。每组样例都包含：

- 一份汇编指令文件 `*.asm`
- 一份输入说明 `input.txt`
- 一份期望结果说明 `expected.txt`
- 一份样例说明 `README.md`
- 一份由 `hpu_encode_cli` 生成的 32 位编码文件 `*.inst32`

## 总表

| 样例 | 目录 | 指令文件 | 输入内容 | 期望结果 | 当前状态 |
|---|---|---|---|---|---|
| NTT | `cases/ntt/` | `ntt.asm` | `p0` 输入多项式，`p2` 模上下文，`p3` shuffle 配置 | 最终结果回到 `p0`，按 `stage=0..5` 完成 6 条 `pntt` | 已可编码，数值黄金值待 twiddle/模上下文格式冻结 |
| INTT | `cases/intt/` | `intt.asm` | `p0` NTT 域输入，`p2` 模上下文，`p3` shuffle 配置 | 最终结果回到 `p0`，按 `stage=0..5` 完成 6 条 `pintt` | 已可编码，数值黄金值待 inverse twiddle/模上下文格式冻结 |
| MM | `cases/mm/` | `mm.asm` | `p0`、`p1` 输入多项式，`p3` 模上下文 | `p2[i] = (p0[i] * p1[i]) mod 257` | 已可编码，已给出可人工验算黄金值 |
| BCONV | `cases/bconv/` | `bconv.asm` | `p0` 输入，`p3`/`p4` 系数对象，`p5`/`p6` 模上下文 | 先得中间结果 `p1`，再得到输出 `p2` | 已可编码，已给出中间值和最终黄金值 |

## 各样例文件清单

### 1. NTT

- [ntt.asm](/home/ubuntu/HPU/encode/cases/ntt/ntt.asm)
- [input.txt](/home/ubuntu/HPU/encode/cases/ntt/input.txt)
- [expected.txt](/home/ubuntu/HPU/encode/cases/ntt/expected.txt)
- [README.md](/home/ubuntu/HPU/encode/cases/ntt/README.md)
- [ntt.inst32](/home/ubuntu/HPU/encode/cases/ntt/ntt.inst32)

用途：
给出 `N=64` 的 stage 级 NTT 指令序列，用于检查 `pmodld`、`pshcfg`、`pntt`、`psync` 的发射顺序和编码结果。

### 2. INTT

- [intt.asm](/home/ubuntu/HPU/encode/cases/intt/intt.asm)
- [input.txt](/home/ubuntu/HPU/encode/cases/intt/input.txt)
- [expected.txt](/home/ubuntu/HPU/encode/cases/intt/expected.txt)
- [README.md](/home/ubuntu/HPU/encode/cases/intt/README.md)
- [intt.inst32](/home/ubuntu/HPU/encode/cases/intt/intt.inst32)

用途：
给出 `N=64` 的 stage 级 INTT 指令序列，用于检查 `pintt` 路径与 NTT 的对称性。

### 3. MM

- [mm.asm](/home/ubuntu/HPU/encode/cases/mm/mm.asm)
- [input.txt](/home/ubuntu/HPU/encode/cases/mm/input.txt)
- [expected.txt](/home/ubuntu/HPU/encode/cases/mm/expected.txt)
- [README.md](/home/ubuntu/HPU/encode/cases/mm/README.md)
- [mm.inst32](/home/ubuntu/HPU/encode/cases/mm/mm.inst32)

用途：
给出最小 `pmul` 样例，便于人工核对编码和结果。输入较小，模数取 `257`，不会发生模回绕。

### 4. BCONV

- [bconv.asm](/home/ubuntu/HPU/encode/cases/bconv/bconv.asm)
- [input.txt](/home/ubuntu/HPU/encode/cases/bconv/input.txt)
- [expected.txt](/home/ubuntu/HPU/encode/cases/bconv/expected.txt)
- [README.md](/home/ubuntu/HPU/encode/cases/bconv/README.md)
- [bconv.inst32](/home/ubuntu/HPU/encode/cases/bconv/bconv.inst32)

用途：
给出最小 `Q1 -> P1` basis conversion 样例，便于检查两阶段 `pmodld + pmul` 执行链路。

## 运行与检查方法

生成 32 位编码：

```bash
./build/hpu_encode_cli cases/ntt/ntt.asm
./build/hpu_encode_cli cases/intt/intt.asm
./build/hpu_encode_cli cases/mm/mm.asm
./build/hpu_encode_cli cases/bconv/bconv.asm
```

查看字段分段：

```bash
./build/asm_file_dump cases/mm/mm.asm
./build/asm_file_dump cases/bconv/bconv.asm
```

## 当前交付边界

- `MM` 和 `BCONV` 已提供明确数值黄金值，可直接用于软件对拍或 RTL 联调。
- `NTT` 和 `INTT` 已提供完整指令、输入槽位和输出校验规则，但公开文档尚未冻结 twiddle 常量表与模上下文对象内部布局，因此暂不提供伪造的数值黄金值。
- 一旦 twiddle 表和模上下文对象格式固定，可直接补充到 [cases/ntt/expected.txt](/home/ubuntu/HPU/encode/cases/ntt/expected.txt) 与 [cases/intt/expected.txt](/home/ubuntu/HPU/encode/cases/intt/expected.txt)。
