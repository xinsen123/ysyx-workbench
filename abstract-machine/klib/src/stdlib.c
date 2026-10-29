#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

void *malloc(size_t size) {
  // PA 中的简化实现: 只分配, 不回收(bump allocator).
  // 从 heap.start 开始往后分配, 每次返回 [addr, addr + size) 并把 addr 后移.
  static uintptr_t addr = 0;

  if (addr == 0) addr = (uintptr_t)heap.start;  // 首次调用时确定堆的起点
  // On native, malloc() may be called during initialization of C runtime,
  // before AM has set up `heap`. Don't panic() here (panic() -> putchar() ->
  // glibc -> malloc() would be a dead recursion), just fail gracefully.
  if (addr == 0) return NULL;

  // 对齐到 16 字节, 满足常见基本类型(含 64 位整数/指针)的对齐要求
  addr = (addr + 15) & ~(uintptr_t)15;
  uintptr_t end = (uintptr_t)heap.end;
  if (addr > end || size > end - addr) return NULL;  // 堆空间不足

  void *ret = (void *)addr;
  addr += size;
  return ret;
}

void free(void *ptr) {
}

#endif
