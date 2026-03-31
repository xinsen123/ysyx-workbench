#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Vtop.h"

Vtop *top = new Vtop;

void update() {
    top->clk = 1;top->eval();
    top->clk = 0;top->eval();
}

int main() {

    time_t last = 0;

    while (1) {
        if (time(NULL) - last) {
            last = time(NULL);
            top->inst = 0;
            update();
        }
    }
}
