#include "include/const.h"

uint8_t pmem[MEM_SIZE];

uint8_t *guest_to_host(int paddr) { return pmem + (uint32_t)paddr - MEM_BASE; }

int host_read(int paddr){
    if(paddr < MEM_BASE || paddr > MEM_BASE + MEM_SIZE) {
        log_err("paddr is out of memory: 0x%x", paddr);
    }
    return *(int *)guest_to_host(paddr);
}

extern "C" int pmem_read(int paddr) {
    return host_read(paddr);
};

extern "C" void pmem_write(int paddr, int wdata, char wmask) {
    // printf("case:%04x->%08x\n", wmask, wdata);
    switch (wmask) {
        case SL_BYTE: *(uint8_t *)  guest_to_host(paddr) = (uint8_t)  wdata; break;
        case SL_HALF: *(uint16_t *) guest_to_host(paddr) = (uint16_t) wdata;break;
        case SL_WORD: *(uint32_t *) guest_to_host(paddr) = (uint32_t) wdata;break;
        default: break;
    }
    return;
};

void load_img(char *img) {
    FILE *fp = fopen(img, "r");
    if (fp == NULL) {
        printf("img is empty\n");
        assert(0);
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    int ret = fread(guest_to_host(MEM_BASE), size, 1, fp);

    fclose(fp);
    return;
}