#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

/* NPC 不使用 Kconfig，由 npc_config.h 替代 include/generated/autoconf.h */
#include <npc_config.h>
#include <macro.h>

#include <assert.h>
#include <stdlib.h>

typedef uint32_t word_t;
typedef int32_t  sword_t;
#define FMT_WORD "0x%08" PRIx32

typedef word_t vaddr_t;
typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
#define FMT_PADDR "0x%08" PRIx32
typedef uint16_t ioaddr_t;

#include <debug.h>

#endif
