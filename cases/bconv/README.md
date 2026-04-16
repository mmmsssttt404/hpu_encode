# BCONV Case

本样例对应最小的 `Q1 -> P1` Basis Conversion 场景，便于联调。

目录内容：

- `bconv.asm`：两阶段 `pmodld + pmul` 指令序列
- `input.txt`：输入对象、上下文对象和系数对象
- `expected.txt`：中间结果 `p1` 和最终结果 `p2`

本例选择 `num_q=1`、`num_p=1`，因此阶段二只需要一次 `pmul`，不会触发 `pmac` 累加路径；如果后续要扩成 `Q2/P2` 以上规模，再新增更复杂的黄金值即可。
