# NPC — 单周期 RV32E 处理器

`vsrc/` 是一颗单周期、定长 32 位指令的 RISC-V **RV32E** 处理器，使用 Verilator
做仿真，用 NEMU 做 difftest 参考设计。

## 1. 指令集覆盖

RV32E = RV32I 的整数指令 + 只有 16 个通用寄存器 (x0–x15)。

| 类别 | 指令 | 备注 |
| --- | --- | --- |
| 上位立即数 | `lui` `auipc` | |
| 跳转 | `jal` `jalr` | 写回 `pc+4`；目标地址最低位清零 |
| 分支 | `beq` `bne` `blt` `bge` `bltu` `bgeu` | 六条都有 |
| 加载 | `lb` `lh` `lw` `lbu` `lhu` | 符号/零扩展在 `lsu` 里做 |
| 存储 | `sb` `sh` `sw` | 宽度决定写入的字节数 |
| 立即数运算 | `addi` `slti` `sltiu` `xori` `ori` `andi` | |
| 立即数移位 | `slli` `srli` `srai` | 用 funct7 区分 `srli`/`srai`；移位量只有低 5 位 |
| 寄存器运算 | `add` `sub` `sll` `slt` `sltu` `xor` `srl` `sra` `or` `and` | `sub`/`sra` 由 funct7[5] 区分 |
| 其它 | `fence`（空操作）、`ebreak`（停机） | |
| **未实现** | `ecall` `mret` 与 Zicsr（`csrrw`/`csrrs`/…） | 需要 CSR + 异常入口，见 §5 |

未实现的编码一律走 `is_illegal`：RTL 通过 DPI-C 回调 `invalid_inst()` 报出
pc 与指令字并置 `NPC_ABORT`，不会静默变成空操作。

## 2. 模块划分

| 文件 | 职责 |
| --- | --- |
| `ifu.sv` | 取指（DPI-C `pmem_read`） |
| `idu.sv` | 译码：控制信号、立即数生成、非法指令检测 |
| `exu.sv` | ALU 运算 + 分支条件判断 |
| `lsu.sv` | 访存：读回后按宽度做符号/零扩展，写存按宽度传字节数 |
| `top.sv` | 连线、下一条 PC 选择、写回数据选择 |
| `RegisterFile.v` | 16×32 寄存器堆，x0 恒为 0（DPI-C `read_reg` 供 difftest 读） |
| `npc_const.sv` | 数据宽度、`REG_ADDR_WIDTH`、访存类型、写回来源 |
| `npc_isa.sv` | opcode、ALU 操作类型、操作数来源、分支类型 |

控制信号由 `idu` 的 `always_comb` 用 `case (opcode)` + 内层 `case (funct3)`
产生；`is_illegal` 初值为 1，只有**完整译码成功**的分支才清零，这样任何一个
漏掉的 `funct3`/`funct7` 组合都不会被当成合法指令。

### 关键编码事实（写 RTL 前先确认过）

RV32E 沿用 RV32I 的编码，只是 5 bit 寄存器说明符字段的最高位恒为 0，所以取
字段的低 4 位就等于寄存器号：

```
rd  = inst[11:7]   -> 用 inst[10:7]
rs1 = inst[19:15]  -> 用 inst[18:15]
rs2 = inst[24:20]  -> 用 inst[23:20]
```

**但立即数的位域位置不受 RV32E 影响**，必须按 RV32I 的完整位置取，例如 S 型
立即数的低 5 位是满的 `inst[11:7]`（不是 4 位）。

实测：`addi a0, a1, 5` → `0x00558513`，解出 `rs1 = inst[18:15] = 11 = a1`、
`rd = inst[10:7] = 10 = a0`，与工具链输出一致。

## 3. 构建与回归

```bash
cd npc && make                       # verilator 构建，产物 build/obj_dir/Vtop

cd am-kernels/tests/cpu-tests
make ARCH=riscv32e-npc run           # 跑全部 35 个 cpu-tests
```

`ARCH=riscv32e-npc` 会走 `abstract-machine/scripts/riscv32e-npc.mk`
（`-march=rv32e_zicsr -mabi=ilp32e`），M 扩展的乘除法由
`riscv/npc/libgcc/` 里的软件实现提供。

### difftest 的前提

`difftest_regcpy` 在两边直接按各自的 `sizeof(CPU_state)` 做 `memcpy`，
**没有长度协商**，所以 NPC 的 16 个寄存器必须配 NEMU 的 `CONFIG_RVE=y`：

```bash
make -C nemu riscv32e_defconfig      # 打开 CONFIG_RVE
make -C nemu SHARE=1                 # 重编 difftest 参考 so
```

`npc/Makefile` 的 `check-ref` 会在 `ISA != riscv32` 时检查 NEMU 的
`CONFIG_RVE`，不匹配就直接报错退出，而不是静默跑出错误结果。

## 4. 递进验证

除了"35 个测试全 PASS"，还需要一个**必然失败**的对照，否则无法区分"真的通过"
和"检查根本没生效"：

```bash
# 把 exu.sv 的 ALU_SRA 改成逻辑右移，只动一个字符
ALU_SRA : alu_out = input1 >> input2[4:0];       # 变异
make && make -f Makefile.shift ARCH=riscv32e-npc run
# -> [0x800000a0]Diff find in reg [a0]: right: 0xfcc3b2a1, now: 0x04c3b2a1
```

`shift` 立刻报出 difftest 寄存器不一致，说明比对链路是活的。

## 5. 下一步

- **Zicsr + trap**：`mtvec`/`mepc`/`mcause`/`mstatus`/`mscratch`、`csrrw/s/c`
  及立即数形式、`ecall` 异常入口、`mret`，这样 AM 的 `cte.c`/`trap.S` 才能跑。
  目前这些指令会走 `is_illegal`。注意 NEMU 这边的 `isa_raise_intr()` 还是
  PA3 的 TODO 桩、也没有 CSR 指令，所以要先补 NEMU 才能用 difftest 验证。
- 当前 `ifu`/`lsu` 都是组合逻辑直连 DPI-C（单周期），换成总线接口后即可接
  真正的指令/数据存储器。
