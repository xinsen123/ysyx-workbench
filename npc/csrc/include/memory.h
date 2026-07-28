#ifndef MEMORY_H
#define MEMORY_H


#include <stdint.h>
#include <stdio.h>
#include <svdpi.h>
#include <assert.h>

#define MEM_SIZE 0x8000000
#define MEM_BASE 0x80000000

#define SL_BYTE 0x01
#define SL_HALF 0x03
#define SL_WORD 0x0f

uint8_t *guest_to_host(int paddr);

void load_img(char *img);

#endif