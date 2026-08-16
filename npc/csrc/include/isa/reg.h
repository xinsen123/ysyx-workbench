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

#ifndef __NPC_ISA_REG_H__
#define __NPC_ISA_REG_H__

#include <common.h>

/* NPC 的 GPR 在 RTL 中, 通过 DPI-C 导出 task read_reg 读取 (见 main.cpp) */
#ifdef __cplusplus
extern "C" {
#endif
uint32_t npc_read_reg(int idx);
#ifdef __cplusplus
}
#endif

#define gpr(idx) (npc_read_reg(idx))

static inline const char* reg_name(int idx) {
  extern const char* regs[];
  return regs[idx];
}

#endif
