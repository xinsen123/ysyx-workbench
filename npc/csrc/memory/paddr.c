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

#include <isa.h>
#include <memory/host.h>
#include <memory/paddr.h>

/* pmem 数组与 guest_to_host 在 memory/memory.cpp 中 (供 RTL DPI-C 使用) */

static word_t pmem_read(paddr_t addr, int len) {
    word_t ret = host_read(guest_to_host(addr), len);
    return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
    host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
    panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
          addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

word_t paddr_read(paddr_t addr, int len) {
    /* 注意: NEMU 原版把 mtrace 的 log_write 放在越界检查之前, 而且 log 中会先调用
     * 一次 pmem_read, 当 RTL 由于 bug 访问到非法地址时会直接段错误而不是触发 panic,
     * 不利于调试. 这里先做越界检查再记录 trace. */
    word_t data;
    if (likely(in_pmem(addr))) {
        data = pmem_read(addr, len);
    } else {
        out_of_bound(addr);
        return 0;
    }
#ifdef CONFIG_MTRACE_COND
    if (MTRACE_COND) {
        log_write("paddr_read: addr = 0x%08x, len = %1d, data = %08x\n", addr, len, data);
    }
#endif
    return data;
}

void paddr_write(paddr_t addr, int len, word_t data) {
#ifdef CONFIG_MTRACE_COND
    if (MTRACE_COND) {
        log_write("paddr_writ: addr = 0x%08x, len = %1d, data = %08x\n", addr, len, data);
    }
#endif

    if (likely(in_pmem(addr))) {
        pmem_write(addr, len, data);
        return;
    }
    out_of_bound(addr);
}
