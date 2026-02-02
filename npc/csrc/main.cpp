#include <assert.h>
#include <cstddef>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../build/obj_dir/Vtop.h"
#include "verilated.h"
#include <nvboard.h>

VerilatedContext *contextp = new VerilatedContext; // 用于保存仿真的时间

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME *top);

int main() {
    time_t last = 0;
    time_t begin = time(NULL) + 10;

    nvboard_bind_all_pins(&dut);
    nvboard_init();

    while (1) {
        if (time(NULL) - last > 0) {
            last = time(NULL);
            printf("a = %d, b = %d, f = %d\n", dut.a, dut.b, dut.f);
            dut.clk = 1;
            dut.eval();
            dut.clk = 0;
        }
        dut.eval();
        nvboard_update();
    }

    nvboard_quit();
}