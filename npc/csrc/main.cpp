#include <assert.h>
#include <cstddef>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../build/obj_dir/Vtop.h"

int main() {
    Vtop *top = new Vtop;

    time_t last = 0;
    top->pc = 0;

    while (1) {
        if (time(NULL) - last) {
            last = time(NULL);
            top->inst = rand();
            top->clk = 1; top->eval();
            top->clk = 0; top->eval();
        }
    }
}