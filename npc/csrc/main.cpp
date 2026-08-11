#include "../build/obj_dir/Vtop.h"
#include "include/const.h"
#include <iomanip>
#include <iostream>
#include <verilated.h>
#include "svdpi.h"


Vtop *top = new Vtop;

extern "C" void read_reg(int idx, int *num);
void display_regs(){
    int num = 0;
    svScope scope = svGetScopeFromName("TOP.top.GPR");
    svSetScope(scope);//这里将仿真目标转换到GPR模块，用于调用里面导出的函数

    for (int i = 0; i < 16; i++) {
        read_reg(i, &num);
        printf("0x%08x ", num);
        if(i % 4 == 3) putchar('\n');
    }
}

void is_exit_bad() {
    svScope scope = svGetScopeFromName("TOP.top.GPR");
    svSetScope(scope);

    int a0 = 1;
    read_reg(10, &a0);

    if (a0) {
        printf("HIT BAD TRAP at 0x%08x\n", top->pc);
    } else {
        printf("HIT GOOD TRAP at 0x%08x\n", top->pc);
    }
    return;
}

void update() {
    top->clk = 1; top->eval();
    top->clk = 0; top->eval();
}

void reset() {
    top->clk = 0; top->eval();
}

bool is_stop = false;

extern "C" void ebreak() {
    is_stop = true;
}

int main(int argc, char **argv) {

    load_img(argv[1]);
    std::cout<<argv[1];

    reset();
    while (!is_stop) {
        update();
    }
    is_exit_bad();
    display_regs();

    delete top;
}