#include <assert.h>
#include <cstddef>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../obj_dir/Vtop.h"
#include "verilated.h"

VerilatedContext *contextp = new VerilatedContext; // 用于保存仿真的时间

#if CONFIG_FST_WAVE_TRACE               // 如果--trace 生成波形文件
#include "verilated_fst_c.h"            //包含头文件
VerilatedFstC *tfp = new VerilatedFstC; // 创建一个波形文件指针
#endif

int main() {
    Vtop *top = new Vtop;

    time_t last = 0;

#if CONFIG_FST_WAVE_TRACE
    contextp->traceEverOn(true); // 启用跟踪
    top->trace(tfp, 99);         // 采样深度为99
    tfp->open("wave.fsp");    // 打开波形文件，文件地址和文件名可以自定义
#endif

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
#if CONFIG_FST_WAVE_TRACE
            tfp->dump(contextp->time()); // 按照时间采样
#endif
        }
    }
#if CONFIG_FST_WAVE_TRACE
    tfp->close(); // 关闭波形文件
#endif
}