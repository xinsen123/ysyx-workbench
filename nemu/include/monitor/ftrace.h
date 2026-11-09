/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __MONITOR_FTRACE_H__
#define __MONITOR_FTRACE_H__

#ifdef CONFIG_FTRACE

#include <common.h>

/* 从 ELF 符号表里提取出来的函数符号, 按 begin 升序排列 */
typedef struct {
  const char *name;     // 指向 ELF 字符串表副本, 生命周期与 NEMU 进程一致
  vaddr_t begin, end;   // 函数的地址区间 [begin, end)
} fun_sym_t;

extern fun_sym_t *fun_table;
extern int fun_cnt;

/* 返回包含 addr 的函数符号, 找不到返回 NULL */
const fun_sym_t *ftrace_find(vaddr_t addr);

#endif // CONFIG_FTRACE

#endif
