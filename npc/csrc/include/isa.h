/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __ISA_H__
#define __ISA_H__

#include <common.h>

/* NPC 的寄存器堆在 RTL (RegisterFile.v) 中, C++ 侧通过 DPI-C 读取,
 * 因此 CPU_state 只保留 pc 镜像 (与 RTL 的 top->pc 同步) */
typedef struct {
  word_t gpr[16];
  vaddr_t pc;
} riscv32_CPU_state;

typedef struct {
  uint32_t inst;
} riscv32_ISADecodeInfo;

typedef riscv32_CPU_state CPU_state;
typedef riscv32_ISADecodeInfo ISADecodeInfo;

#ifdef __cplusplus
extern "C" {
#endif

// reg (cpu-exec.c 中定义, sdb/expr 通过它读取寄存器)
extern CPU_state cpu;
void isa_reg_display();
word_t isa_reg_str2val(const char *name, bool *success);

#ifdef __cplusplus
}
#endif

#endif
