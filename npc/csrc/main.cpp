#include "../build/obj_dir/Vtop.h"
#include "../build/obj_dir/Vtop_top.h"
#include "svdpi.h"
#include "common.h"
#include "utils.h"
#include "cpu/cpu.h"
#include "monitor/monitor.h"
#include "monitor/sdb/sdb.h"
#include <iostream>


Vtop *top = new Vtop;

extern "C" void read_reg(int idx, int *num);


extern "C" uint32_t npc_read_reg(int idx) {
    int num = 0;
    svScope scope = svGetScopeFromName("TOP.top.GPR");
    svSetScope(scope); // 将仿真目标转换到GPR模块，用于调用里面导出的函数
    read_reg(idx, &num);
    return (uint32_t)num;
}

extern "C" uint32_t npc_get_pc()   { return top->pc; }
extern "C" uint32_t npc_get_inst() { return top->top->inst; }


extern "C" void npc_exec_once() {
    top->clk = 1; top->eval();
    top->clk = 0; top->eval();
}

extern "C" void npc_reset() {
    top->clk = 0; top->eval();
}


static bool npc_ebreak = false;

extern "C" void ebreak() {
    npc_ebreak = true;
}

extern "C" int npc_ebreak_happened() {
    if (npc_ebreak) {
        npc_ebreak = false;
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    init_monitor(argc, argv);   // 先加载镜像, 再复位, 保证复位后取到的第一条指令有效
    npc_reset();
    sdb_mainloop();

    delete top;
    return is_exit_status_bad();
}
