#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../build/obj_dir/Vtop.h"
#include "verilated.h"
#include <nvboard.h>

#define CONFIG_FST_WAVE_TRACE 0

VerilatedContext *contextp = new VerilatedContext; // 用于保存仿真的时间

#if CONFIG_FST_WAVE_TRACE               // 如果--trace 生成波形文件
#include "verilated_fst_c.h"            //包含头文件
VerilatedFstC *tfp = new VerilatedFstC; // 创建一个波形文件指针
#endif

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME *top);


#include <sys/time.h>
static uint64_t get_time_internal() {
    // struct timeval now;
    // gettimeofday(&now, NULL);
    // uint64_t us = now.tv_sec * 1000000 + now.tv_usec;
    // return us;
}
// #define time(x) get_time_internal()
// #define time_t uint64_t

int main() {
    Vtop *top = new Vtop;

    time_t last = 0;
    time_t begin = time(NULL) + 10;

#if CONFIG_FST_WAVE_TRACE
    contextp->traceEverOn(true); // 启用跟踪
    top->trace(tfp, 99);         // 采样深度为99
    tfp->open("wave.fst");       // 打开波形文件，文件地址和文件名可以自定义
#endif

    nvboard_bind_all_pins(&dut);
    nvboard_init();

    while (1) {
        if (time(NULL) - last > 0) {
            last = time(NULL);
//             int a = rand() & 1;
//             int b = rand() & 1;
//             top->a = a;
//             top->b = b;
            dut.eval();
            printf("a = %d, b = %d, f = %d\n", top->a, top->b, top->f);
            // assert(top->f == (a ^ b));

//             contextp->timeInc(1); // 推动仿真时间

// #if CONFIG_FST_WAVE_TRACE
//             tfp->dump(contextp->time()); // 按照时间采样
// #endif
        }
        nvboard_update();
        // top->a = 1;
    }

    nvboard_quit();

#if CONFIG_FST_WAVE_TRACE
    tfp->close(); // 关闭波形文件
#endif
}