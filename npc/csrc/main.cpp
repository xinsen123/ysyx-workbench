#include <assert.h>
#include <cstddef>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../obj_dir/Vtop.h"

int main() {
    Vtop *top = new Vtop;

    time_t last = 0;

    while (1) {
        if (time(NULL) - last) {
            last = time(NULL);
            int a = rand() & 1;
            int b = rand() & 1;
            top->a = a;
            top->b = b;
            top->eval();
            printf("a = %d, b = %d, f = %d\n", a, b, top->f);
            assert(top->f == (a ^ b));
        }
    }
}