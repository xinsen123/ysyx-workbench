#include "Vtop.h"
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
    svSetScope(scope);

    for (int i = 0; i < 16; i++) {
        read_reg(i, &num);
        printf("0x%08x ", num);
        if(i % 4 == 3) putchar('\n');
    }
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

    reset();
    int i = 100;
    while (i-- && !is_stop) {
        update();
        // display_regs();
    }

    delete top;
}