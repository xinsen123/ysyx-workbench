#include <utils.h>
#include <cpu/cpu.h>

NPCState npc_state = { .state = NPC_STOP };

int is_exit_status_bad() {
  int good = (npc_state.state == NPC_END && npc_state.halt_ret == 0) ||
    (npc_state.state == NPC_QUIT);
  return !good;
}

/* RTL (idu.sv) 译码失败时通过 DPI-C 调用, 用于把"非法指令"和"程序正常停机"
 * 区分开: 前者是 ABORT, 退出码非 0。 */
void invalid_inst(vaddr_t thispc) {
  printf("invalid instruction at pc = " FMT_WORD ", inst = " FMT_WORD "\n",
         thispc, npc_get_inst());
  npc_state.state = NPC_ABORT;
  npc_state.halt_pc = thispc;
  npc_state.halt_ret = -1;
}
