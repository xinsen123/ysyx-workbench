/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "utils.h"
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <stdio.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 16

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

#ifdef CONFIG_ITRACE
static char iringbuf[MAX_INST_TO_PRINT][128];
static int p_ring = 0;
#endif

#ifdef CONFIG_FTRACE
#include <monitor/ftrace.h>

#define FTRACE_MAX_DEPTH 64

/* 影子调用栈: 进入一个函数压一帧, 返回时弹一帧.
 * ra 是这个函数返回后要回到的地址(即 jal/jalr 写入 rd 的值),
 * name 是这个函数的名字(打印 ret 行时用). */
typedef struct {
  vaddr_t ra;
  const char *name;
} ftrace_frame;

static ftrace_frame ftrace_stack[FTRACE_MAX_DEPTH];
static int ftrace_depth = 0;

/* riscv32 中"调用/返回"的判定只看指令本身, 和跳转目标落在哪个函数无关:
 *   jal  rd, off         (opcode 0x6f)
 *       rd != 0 -> call
 *       rd == 0 -> 无条件跳转(可能是尾调用)
 *   jalr rd, rs1, off    (opcode 0x67, funct3 == 0)
 *       rd == 0 && rs1 == ra(x1) && off == 0 -> ret (伪指令 ret)
 *       rd != 0                              -> call (间接调用也走这里)
 *       rd == 0 其它                         -> 无条件跳转(可能是尾调用)
 * 跳转目标 dnpc 只用来查函数名, 查不到就打印 "???" 而不是丢弃.
 */
static void ftrace(Decode *s, vaddr_t dnpc) {
  uint32_t inst = s->isa.inst;
  uint32_t opcode = inst & 0x7f;
  bool is_jal  = (opcode == 0x6f);
  bool is_jalr = (opcode == 0x67) && (((inst >> 12) & 0x7) == 0);
  if (!is_jal && !is_jalr) return;

  uint32_t rd  = (inst >> 7) & 0x1f;
  uint32_t rs1 = (inst >> 15) & 0x1f;
  int32_t  imm = (int32_t)inst >> 20;

  bool is_ret = is_jalr && rd == 0 && rs1 == 1 && imm == 0;

  if (is_ret) {
    if (ftrace_depth == 0) {
      if (FTRACE_COND) {
        log_write("[ftrace] " FMT_WORD ": ret  [???] (call stack empty)\n", s->pc);
      }
      return;
    }
    ftrace_depth--;
    if (FTRACE_COND) {
      log_write("[ftrace] " FMT_WORD ": %*sret  [%s] ra=" FMT_WORD "\n",
          s->pc, ftrace_depth * 2, "",
          ftrace_stack[ftrace_depth].name, ftrace_stack[ftrace_depth].ra);
    }
    return;
  }

  const fun_sym_t *sym = ftrace_find(dnpc);
  const char *name = sym ? sym->name : "???";

  if (rd != 0) {  // call
    if (FTRACE_COND) {
      log_write("[ftrace] " FMT_WORD ": %*scall [%s@" FMT_WORD "]\n",
          s->pc, ftrace_depth * 2, "", name, dnpc);
    }
    if (ftrace_depth < FTRACE_MAX_DEPTH) {
      ftrace_stack[ftrace_depth].ra = s->snpc;  // jal/jalr 写入 rd 的返回地址
      ftrace_stack[ftrace_depth].name = name;
      ftrace_depth++;
    }
    return;
  }

  /* rd == 0 且不是 ret: 无条件跳转. 只有目标是另一个函数的入口时才算尾调用,
   * 函数内部的 goto/循环跳转不算. */
  const fun_sym_t *cur = ftrace_find(s->pc);
  if (sym == NULL || (cur != NULL && sym->begin == cur->begin)) return;

  if (ftrace_depth > 0) {
    /* 尾调用不产生新栈帧: 把当前帧换成被调用的函数, ra 保持不变 */
    ftrace_stack[ftrace_depth - 1].name = name;
    if (FTRACE_COND) {
      log_write("[ftrace] " FMT_WORD ": %*stail [%s@" FMT_WORD "]\n",
          s->pc, (ftrace_depth - 1) * 2, "", name, dnpc);
    }
  } else if (FTRACE_COND) {
    log_write("[ftrace] " FMT_WORD ": tail [%s@" FMT_WORD "]\n", s->pc, name, dnpc);
  }
}
#endif

void device_update();
void is_wp_update(bool *success);

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) {
        log_write("[itrace] %s\n", _this->logbuf);
        snprintf(iringbuf[(p_ring++) % MAX_INST_TO_PRINT], 128, "%s", _this->logbuf);
    }
#endif
    if (g_print_step) {
        IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
    }
    IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));

#ifdef CONFIG_WATCHPOINT
    bool success = false;
    is_wp_update(&success);
    if (success == true) {
        nemu_state.state = NEMU_STOP;
    }
#endif

#ifdef CONFIG_FTRACE
    ftrace(_this, dnpc);
#endif
}

static void exec_once(Decode *s, vaddr_t pc) {
    s->pc = pc;
    s->snpc = pc;
    isa_exec_once(s);
    cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
    char *p = s->logbuf;
    p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);  //利用指针p给logbuf所指内存赋值
    int ilen = s->snpc - s->pc;
    int i;
    uint8_t *inst = (uint8_t *)&s->isa.inst;
#ifdef CONFIG_ISA_x86
    for (i = 0; i < ilen; i++) {
#else
    for (i = ilen - 1; i >= 0; i--) {
#endif
        p += snprintf(p, 4, " %02x", inst[i]);
    }
    int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4); //在编译期选择参数
    int space_len = ilen_max - ilen;
    if (space_len < 0) space_len = 0;
    space_len = space_len * 3 + 1;
    memset(p, ' ', space_len);
    p += space_len;

    void disassemble(char *str, int size, uint64_t pc, uint8_t *code,
                     int nbyte);
    disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
                MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst,
                ilen);
#endif
}

static void execute(uint64_t n) {
    Decode s;
    for (; n > 0; n--) {
        exec_once(&s, cpu.pc);
        g_nr_guest_inst++;
        trace_and_difftest(&s, cpu.pc);
        if (nemu_state.state != NEMU_RUNNING) break;
        IFDEF(CONFIG_DEVICE, device_update());
    }
}

static void statistic() {
    IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
    Log("host time spent = " NUMBERIC_FMT " us", g_timer);
    Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
    if (g_timer > 0)
        Log("simulation frequency = " NUMBERIC_FMT " inst/s",
            g_nr_guest_inst * 1000000 / g_timer);
    else
        Log("Finish running in less than 1 us and can not calculate the "
            "simulation frequency");
}

void assert_fail_msg() {
    isa_reg_display();
    statistic();
}

#ifdef CONFIG_ITRACE

static void ringbuf_print(){
    for(int i = 0; i < MAX_INST_TO_PRINT && iringbuf[i][0] != '\0'; i++){
        log_write("[iringbuf] %s%s\n", ((p_ring - 1) % MAX_INST_TO_PRINT) == i ? "--> " : "    ", iringbuf[i]);
        printf("[iringbuf] %s%s\n", ((p_ring - 1) % MAX_INST_TO_PRINT) == i ? "--> " : "    ", iringbuf[i]);
    }
}

#endif

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
    g_print_step = (n < MAX_INST_TO_PRINT);
    switch (nemu_state.state) {
    case NEMU_END:
    case NEMU_ABORT:
    case NEMU_QUIT:
        printf("Program execution has ended. To restart the program, exit NEMU "
               "and run again.\n");
               return;
    default:
        nemu_state.state = NEMU_RUNNING;
    }

    uint64_t timer_start = get_time();

    execute(n);

    uint64_t timer_end = get_time();
    g_timer += timer_end - timer_start;

    switch (nemu_state.state) {
    case NEMU_RUNNING:
        nemu_state.state = NEMU_STOP;
        break;

    case NEMU_END:
    case NEMU_ABORT:
        Log("nemu: %s at pc = " FMT_WORD,
            (nemu_state.state == NEMU_ABORT
                 ? ANSI_FMT("ABORT", ANSI_FG_RED)
                 : (nemu_state.halt_ret == 0
                        ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN)
                        : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
            nemu_state.halt_pc);
    #ifdef CONFIG_ITRACE
            ringbuf_print();
    #endif
        // fall through
    case NEMU_QUIT:
        statistic();
    }
}
