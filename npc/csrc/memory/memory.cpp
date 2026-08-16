/* NPC 的物理内存 + 与 RTL 交互的 DPI-C 接口
 * 原 csrc/memory.cpp 保留了 pmem 与 DPI-C 函数,
 * 读写统一走 paddr_read/paddr_write (memory/paddr.c), 从而 mtrace 可以记录 RTL 的所有访存
 */
#include "const.h"
#include <memory/paddr.h>

uint8_t pmem[MEM_SIZE];

extern "C" uint8_t *guest_to_host(paddr_t paddr) { return pmem + (uint32_t)paddr - MEM_BASE; }

extern "C" paddr_t host_to_guest(uint8_t *haddr) { return (paddr_t)(haddr - pmem + MEM_BASE); }

/* RTL ifu.v 取指: 按 4 字节读 */
extern "C" int pmem_read(int paddr) {
    return paddr_read((paddr_t)paddr, 4);
};

/* RTL lsu.v 写存: wmask 与长度的对应关系见 const.vh 的 SL_* 定义 */
extern "C" void pmem_write(int paddr, int wdata, char wmask) {
    switch (wmask) {
        case SL_BYTE: paddr_write((paddr_t)paddr, 1, (word_t)wdata); break;
        case SL_HALF: paddr_write((paddr_t)paddr, 2, (word_t)wdata); break;
        case SL_WORD: paddr_write((paddr_t)paddr, 4, (word_t)wdata); break;
        default: break;
    }
    return;
};
