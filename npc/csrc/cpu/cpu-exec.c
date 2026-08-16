#include <utils.h>
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <isa/reg.h>
#include <memory/paddr.h>
#include <locale.h>
#include <stdio.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 16

CPU_state cpu = { .pc = RESET_VECTOR }; // 与 RTL 的复位 pc 保持一致
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

#ifdef CONFIG_ITRACE
static char iringbuf[MAX_INST_TO_PRINT][128];
static int p_ring = 0;
#endif

#ifdef CONFIG_FTRACE
static int fun_indent = 0;
typedef struct {
    char name[32];
    uint32_t begin, end;
} elf_obj;
/* 引用 monitor.c 中定义的变量 */
extern elf_obj fun_table[128];
extern int fun_cnt;
#endif

void is_wp_update(bool *success);

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) {
        log_write("%s\n", _this->logbuf);
        snprintf(iringbuf[(p_ring++) % MAX_INST_TO_PRINT], 128, "%s", _this->logbuf);
    }
#endif
    if (g_print_step) {
        IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
    }
    /* difftest 由你自己实现: 在 NPC 的每个周期结束后调用
     * IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc)); */

#ifdef CONFIG_WATCHPOINT
    bool success = false;
    is_wp_update(&success);
    if (success == true) {
        npc_state.state = NPC_STOP;
    }
#endif

#ifdef CONFIG_FTRACE
    if ((((_this->isa.inst & 0x7f) == 0x6f) && ((_this->isa.inst & 0xf80) != 0x0)) ||//jal，rd不为x0时有意义
    (((_this->isa.inst & 0x7f) == 0x67) && (_this->isa.inst & 0x7000) == 0x0)) {//opcode & fun3 -> jalr
        for(int i = 0; i < fun_cnt; i++) {
            /* rd不为x0的jal和任意jalr可识别为函数出入口，然后再对jalr进行进一步判断 */
            if(dnpc >= fun_table[i].begin && dnpc < fun_table[i].end){
                bool is_ret = ((_this->isa.inst & 0xf80) == 0x0) &&    // rd == x0
                              ((_this->isa.inst & 0xf8000) == 0x8000); // rs1 == ra(x1)
                const char *ret_word = is_ret ? "ret" : "call";
                fun_indent = is_ret ? fun_indent > 0 ? fun_indent - 1 : fun_indent : fun_indent + 1;
                printf("0x%8x: %*s%s[%s@0x%8x]\n", _this->pc, fun_indent * 2, "", ret_word, fun_table[i].name, dnpc);
                break;
            }
        }
    }
#endif
}



static void exec_once(Decode *s, vaddr_t pc) {
    s->pc = pc;
    s->snpc = pc + 4;
    s->isa.inst = npc_get_inst();

    npc_exec_once();

    s->dnpc = npc_get_pc();
    cpu.pc = s->dnpc;

    /* RTL 在 ebreak 指令处调用 DPI-C ebreak(), 这里补全 NPC NPCTRAP 的语义 */
    if (npc_ebreak_happened()) {
        npc_state.state = NPC_END;
        npc_state.halt_pc = s->pc;
        npc_state.halt_ret = npc_read_reg(10); // R(10) is $a0
    }

#ifdef CONFIG_ITRACE
    char *p = s->logbuf;
    p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);  //利用指针p给logbuf所指内存赋值
    int ilen = s->snpc - s->pc;
    int i;
    uint8_t *inst = (uint8_t *)&s->isa.inst;
    for (i = ilen - 1; i >= 0; i--) {
        p += snprintf(p, 4, " %02x", inst[i]);
    }
    int ilen_max = 4; // NPC 为定长 32 位指令
    int space_len = ilen_max - ilen;
    if (space_len < 0) space_len = 0;
    space_len = space_len * 3 + 1;
    memset(p, ' ', space_len);
    p += space_len;

    /* capstone 反汇编 (utils/disasm.c) */
    void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
    disassemble(p, s->logbuf + sizeof(s->logbuf) - p, s->pc,
                (uint8_t *)&s->isa.inst, ilen);
#endif
}

static void execute(uint64_t n) {
    Decode s;
    for (; n > 0; n--) {
        exec_once(&s, cpu.pc);
        g_nr_guest_inst++;
        trace_and_difftest(&s, cpu.pc);
        if (npc_state.state != NPC_RUNNING) break;
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
        printf("%s%s\n", (p_ring % MAX_INST_TO_PRINT) == i + 1 ? "--> " : "    ", iringbuf[i]);
    }
}
#endif
/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
    g_print_step = (n < MAX_INST_TO_PRINT);
    switch (npc_state.state) {
    case NPC_END:
    case NPC_ABORT:
    case NPC_QUIT:
        printf("Program execution has ended. To restart the program, exit NPC "
               "and run again.\n");
        #ifdef CONFIG_ITRACE
               ringbuf_print();
        #endif
        return;
    default:
        npc_state.state = NPC_RUNNING;
    }

    uint64_t timer_start = get_time();

    execute(n);

    uint64_t timer_end = get_time();
    g_timer += timer_end - timer_start;

    switch (npc_state.state) {
    case NPC_RUNNING:
        npc_state.state = NPC_STOP;
        break;

    case NPC_END:
    case NPC_ABORT:
        Log("npc: %s at pc = " FMT_WORD,
            (npc_state.state == NPC_ABORT
                 ? ANSI_FMT("ABORT", ANSI_FG_RED)
                 : (npc_state.halt_ret == 0
                        ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN)
                        : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
            npc_state.halt_pc);
        #ifdef CONFIG_ITRACE
               ringbuf_print();
        #endif
        // fall through
    case NPC_QUIT:
        statistic();
    }
}
