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
#include "macro.h"
#include <assert.h>
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    TK_NOTYPE = 256, // none
    TK_EQ,           // ==
    TK_NE,           // ！=
    TK_ST,           // <=
    TK_BT,           // >=
    TK_AND,          // &&
    TK_OPPOSITE,     // opposite
    TK_DNUM,         // full 10 add 1
    TK_XNUM,         // full 16
    TK_POINTER,

    /* TODO: Add more token types */

};

static struct rule {
    const char *regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\+", '+'},      // plus

    {"\\-", '-'},           // sub
    {"TK_OP", TK_OPPOSITE}, // opposite, must after of '-'

    {"\\*", '*'}, // mul
    {"TK_PO", TK_POINTER},

    {"/", '/'},    // div
    {"\\(", '('},  // (
    {"\\)", ')'},  // )
    {"==", TK_EQ}, // equal
    {"!=", TK_NE},
    {"<=", TK_ST},
    {">=", TK_BT},
    {"&&", TK_AND},
    {"0x[0-9]+", TK_XNUM},
    {"[0-9]+", TK_DNUM},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */

void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg,
                  rules[i].regex);
        }
    }
}

typedef struct token {
    int type;
    char str[32];
} Token;

static Token tokens[1024]
    __attribute__((used)) = {}; //__attribute__ do not clear it even if not used
static int nr_token __attribute__((used)) = 0;

/*those are indivial values*/
static int oppo_f_type[] = {'+',   '-',   '*',   '/',   '(',
                            TK_ST, TK_BT, TK_EQ, TK_NE, TK_AND};

static int op_1th[] = {TK_EQ, TK_NE, TK_ST, TK_BT, TK_AND};
static int op_2th[] = {'+', '-'};
static int op_3th[] = {'*', '/'};

static int ambi_type[] = {'-', '*'};

bool is_type(int type, int *type_list, uint32_t size) {
    for (int i = 0; i < size; i++) {
        if (type == type_list[i]) return true;
    }
    return false;
}

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0' && e[position] != '\n') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {

            while (e[position] == ' ') // remove space
                position++;

            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
                pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                // if (rules[i].token_type == '-') {
                //     if (nr_token == 0) i++;
                //     else if (is_type(tokens[nr_token - 1].type, oppo_f_type,
                //     sizeof(oppo_f_type)))
                //         i++;
                // }

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len,
                    substr_start);

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                memset(tokens[nr_token].str, 0, 32); // 预防残留

                switch (rules[i].token_type) {
                case TK_DNUM:
                case TK_XNUM:
                    strncpy(tokens[nr_token].str, e + position, substr_len);
                    tokens[nr_token].str[31] = '\0';
                default:
                    tokens[nr_token].type = rules[i].token_type;
                    nr_token++;
                }
                position += substr_len;

                while (e[position] == ' ') // remove space
                    position++;

                break;
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e,
                   position, "");
            return false;
        }
    }

    return true;
}

bool check_parentheses(uint32_t p, uint32_t q) {
    if (tokens[p].type != '(' || tokens[q].type != ')') return false;
    int i, balance = 0;

    for (i = p; i < q; i++) {
        if (tokens[i].type == '(') balance++;
        else if (tokens[i].type == ')') balance--;
        if (balance == 0) return false;
    }

    return true;
}

int eval(uint32_t p, uint32_t q);

int main_sign(int p, int q, int *op_list) {

    int i, j, pos, parent_count;
    int size = sizeof(&op_list);
    for (i = q; i >= p; i--) {
        for (pos = 0; pos < size; pos++) {
            if (tokens[i].type == op_list[pos]) {
                if (is_type(tokens[i].type, ambi_type, sizeof(ambi_type)) ==
                    true) {
                    if (i == p || is_type(tokens[i - 1].type, oppo_f_type,
                                          sizeof(oppo_f_type))) {
                        continue;
                    }
                }

                parent_count = 0;
                for (j = q; j > i; j--) {
                    if (tokens[j].type == '(') parent_count++;
                    if (tokens[j].type == ')') parent_count--;
                }
                if (parent_count == 0) return i;
            }
        }
    }
    return 0;
}

// static int nums[] = {TK_XNUM, TK_DNUM};

static int eval_op_eval(uint32_t p, uint32_t q) {
    uint32_t op;
    op = main_sign(p, q, op_1th);
    if (op == 0) op = main_sign(p, q, op_2th); // 运算具有优先级顺序
    if (op == 0) op = main_sign(p, q, op_3th);

    Assert(op != 0, "Invalid expr");

    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);

    switch (tokens[op].type) {
    case '+':
        return val1 + val2;
    case '-':
        return val1 - val2;
    case '*':
        return val1 * val2;
    case '/':
        Assert(val2 != 0, "0 can't be div");
        return val1 / val2;
    case TK_EQ:
        return val1 == val2 ? 1 : 0;
    case TK_NE:
        return val1 != val2 ? 1 : 0;
    case TK_ST:
        return val1 <= val2 ? 1 : 0;
    case TK_BT:
        return val1 >= val2 ? 1 : 0;
    case TK_AND:
        return val1 && val2 ? 1 : 0;
    default:
        panic("Cannot recognise the opsign");
    }
}

int eval(uint32_t p, uint32_t q) {
    if (p > q) { /* Bad expression */
        panic("Error occured in evaluate");
    } else if (p == q) { /* Single token.
                          * For now this token should be a number.
                          * Return the value of the number.
                          */
        return atoi(tokens[p].str);

    } else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses.
         */
        return eval(p + 1, q - 1);
    } else if (is_type(tokens[p].type, ambi_type, sizeof(ambi_type))) {
        return eval(p + 1, q);
    }
    return eval_op_eval(p, q);
}

word_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */

    printf("nr_token: %d\n", nr_token);
    return eval(0, nr_token - 1);
}
