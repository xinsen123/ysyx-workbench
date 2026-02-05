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
#include "sdb.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NR_WP 32

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
    WP *new = free_;
    free_ = free_->next;

    new->next = head;
    if (head != NULL) head->next = NULL;
    head = new;

    strcpy(head->name, name);
    head->addr = addr;
    return;
};
void free_wp(WP *wp);

void show_wp() {
    WP *now = head;
    while (now != NULL) {
        printf("%-8d: 0x%8x\n", now->NO, now->addr);
        now = now->next;
    }
}
/* TODO: Implement the functionality of watchpoint */
