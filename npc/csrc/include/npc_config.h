/***************************************************************************************
 * NPC 的配置头文件
 * 对应 NEMU 中由 Kconfig 生成的 include/generated/autoconf.h
 * 需要新加/关闭某个功能时，在这里增删宏即可
 ***************************************************************************************/

#ifndef __NPC_CONFIG_H__
#define __NPC_CONFIG_H__

/* 物理内存布局 */
#define CONFIG_MBASE 0x80000000u
#define CONFIG_MSIZE 0x8000000u
#define CONFIG_PC_RESET_OFFSET 0

/* 访客 ISA */
#define __GUEST_ISA__ riscv32

#define CONFIG_ISA_riscv 1

/* 是否输出 trace 到日志文件 (log_write 的开关) */

#define CONFIG_TRACE 1
#define CONFIG_TRACE_START 1
#define CONFIG_TRACE_END 0xffffffffu


/* itrace: 指令执行 trace (默认全开) */

#define CONFIG_ITRACE 1
#define CONFIG_ITRACE_COND 1
#define ITRACE_COND 1


/* mtrace: 访存 trace */

// #define CONFIG_MTRACE 1
// #define CONFIG_MTRACE_COND 1
// #define MTRACE_COND 1


/* ftrace: 函数调用 trace */

#define CONFIG_FTRACE 1


/* sdb 监视点 */

#define CONFIG_WATCHPOINT 1


/* 供 utils.h 的 log_write 使用 */

#define CONFIG_TARGET_NATIVE_ELF 1

#endif
