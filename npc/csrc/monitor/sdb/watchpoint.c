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
#include "isa.h"
#include "memory/paddr.h"
#include "sdb.h"
#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NR_WP 32

word_t vaddr_read(vaddr_t addr, int len);

typedef struct watchpoint {
    int NO;
    struct watchpoint *next;
    uint32_t num;
    char name[32];

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
void new_wp(char *args) {
    Assert(free_ != NULL, "free pool is full");

    WP *wn = head;
    if (wn == NULL) {
        wn = free_;
        head = wn;
    } else {
        while (wn->next != NULL)
            wn = wn->next;
        wn->next = free_;
        wn = wn->next;
    }

    free_=free_->next;

    wn->next = NULL;
    bool sc = false;
    wn->num = expr(args, &sc);
    if (sc == false) {
        printf("invaild expr");
        return;
    }
    strncpy(wn->name, args, 32);
};
void free_wp(int NO) {
    if (NO < 0 || NO > 31) {
        printf("cannot find wp");
        return;
    }

    WP *wn = (WP *)malloc(sizeof(WP));
    WP *rubb = wn;
    wn->next = head;
    while (wn->next != NULL) {
        if (wn->next->NO == NO) {
            if (wn->next == head) head = wn->next->next;

            WP *ret = wn->next;
            wn->next = wn->next->next;

            ret->next = NULL;
            strncpy(ret->name, "\0", 32);
            ret->num = 0;

            wn = free_;
            Assert(wn != NULL, "wp free pool is nothing");
            while (wn->next != NULL) {
                wn = wn->next;
            }
            wn->next = ret;
            free(rubb);
            return;
        }
        wn = wn->next;
    }
    free(rubb);
    return;
}

void show_wp() {
    WP *now = head;
    printf("%-8s| %-10s| %-32s\n", "NO", "Num", "Name");
    while (now != NULL) {
        printf("%-8d| 0x%-8x| %-32s\n", now->NO, now->num, now->name);
        now = now->next;
    }
}
/* TODO: Implement the functionality of watchpoint */

void is_wp_update(bool *success) {
    if (head == NULL) {
        *success = false;
        return;
    }
    WP *wn = head;
    while (wn != NULL) {
        bool sc;
        int no_num = expr(wn->name, &sc);
        if (wn->num != no_num) {
            *success = true;
            printf("watchpoint updated: %d %s: %x -> %x\n", wn->NO, wn->name,
                   wn->num, no_num);
            wn->num = no_num;
        }
        wn = wn->next;
    }
    return;
}
