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

#include "debug.h"
#include "memory/paddr.h"
#include "sdb.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define NR_WP 32

word_t vaddr_read(vaddr_t addr, int len);

typedef struct watchpoint {
    int NO;
    struct watchpoint *next;
    uint32_t num;
    char name[32];
    uint32_t addr;

    /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    }

    head = NULL;
    free_ = wp_pool;
}
void new_wp(char *name, uint32_t addr) {
    Assert(free_ != NULL, "free pool is full");
    if (addr > PMEM_RIGHT || addr < PMEM_LEFT) {
        printf("addr is out of bound\n");
        return;
    }

    WP *new = head;
    if (new == NULL) {
        new = free_;
        head = new;
    } else {
        while (new->next != NULL)
            new = new->next;
        new->next = free_;
        new = new->next;
    }

    free_ = free_->next;
    new->next = NULL;

    strncpy(new->name, name, 32);
    new->addr = addr;
    new->num = vaddr_read(addr & ~0x3, 4);
    return;
};
void free_wp(int NO) {
    if (NO < 0 || NO > 31) {
        printf("cannot find wp");
        return;
    }

    WP *new = head;
    Assert(new != NULL, "wp work pool is nothing");
    while (new->next != NULL) {
        if (new->next->NO == NO) {
            WP *ret = new->next;
            new->next = new->next->next;

            ret->next = NULL;
            strncpy(ret->name, "\0", 32);
            ret->addr = 0;

            new = free_;
            Assert(new != NULL, "wp free pool is nothing");
            while (new->next != NULL) {
                new = new->next;
            }
            new->next = ret;
        }
        new = new->next;
    }
}

void show_wp() {
    WP *now = head;
    printf("%-8s| %-10s| %-12s| %-32s\n", "NO", "Address", "Num", "Name");
    while (now != NULL) {
        printf("%-8d| 0x%-8x| %-12u| %-32s\n", now->NO, now->addr, now->num,
               now->name);
        now = now->next;
    }
}
/* TODO: Implement the functionality of watchpoint */
