#ifndef __CPU_CPU_H__
#define __CPU_CPU_H__

#include <common.h>

/* NPC 与 RTL 的交互接口 (main.cpp 中实现, 供 C 侧 cpu-exec.c 调用) */
#ifdef __cplusplus
extern "C" {
#endif
void npc_reset();
void npc_exec_once();           /* 一个时钟周期 = 执行一条指令 */
uint32_t npc_get_pc();          /* 当前 RTL 中的 pc */
uint32_t npc_get_inst();        /* 当前 RTL 取到的指令 (本周期执行的指令) */
int npc_ebreak_happened();      /* 返回 1 并清除 ebreak 标志 */
#ifdef __cplusplus
}
#endif

void cpu_exec(uint64_t n);

void set_nemu_state(int state, vaddr_t pc, int halt_ret);
void invalid_inst(vaddr_t thispc);

#define NEMUTRAP(thispc, code) set_nemu_state(NEMU_END, thispc, code)
#define INV(thispc) invalid_inst(thispc)

#endif
