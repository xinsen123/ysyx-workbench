#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEM_SIZE 0x8000000
#define MEM_BASE 0x80000000


/* RTL (ifu.v/lsu.v) 通过 DPI-C 调用的访存接口, 在 memory/memory.cpp 中实现 */
#ifdef __cplusplus
extern "C" {
#endif
int pmem_read(int paddr);
void pmem_write(int paddr, char wmask, int wdata);
#ifdef __cplusplus
}
#endif

#endif
