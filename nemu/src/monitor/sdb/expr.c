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
    TK_NOTYPE = 256,
    TK_EQ,
    TK_DNUM, // full 10 add 1
    TK_XNUM, // full 16

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
    {"==", TK_EQ},     // equal
    {"\\-", '-'},      // sub
    {"\\*", '*'},      // mul
    {"/", '/'},        // div
    {"\\(", '('},      // (
    {"\\)", ')'},      // )
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

static Token tokens[32]
    __attribute__((used)) = {}; //__attribute__ do not clear it even if not used
static int nr_token __attribute__((used)) = 0;

int ptoken = 0;

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
                pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len,
                    substr_start);

                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                switch (rules[i].token_type) {
                case TK_DNUM:
                case TK_XNUM:
                    strncpy(tokens[ptoken].str, e + position, substr_len);
                    tokens[ptoken].str[31] = '\0';
                default:
                    tokens[ptoken].type = rules[i].token_type;
                    ptoken++;
                }

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
    int i, left_num = 0, right_num = 0,
           max_num = 0; // 检测括号存在的峰值和总数量，若峰值!=总数量则有问题
    for (i = p; i <= q; i++) {
        if (tokens[i].type == '(')
            left_num++;
        if (tokens[i].type == ')')
            right_num--;
        max_num =
            max_num < left_num - right_num ? left_num - right_num : max_num;
    }
    if (left_num - right_num != 0 || max_num != left_num) {
        return false;
    } else {
        return true;
    }
}

int main_sign(int p, int q, char a, char b) {

    int i = 0, left = 0, right = 0;

    for (i = p; i <= q; i++) {
        left = tokens[i].type == '(' ? left + 1 : left;
        right = tokens[i].type == ')' ? right + 1 : right;
        if (left == right) {
            if (tokens[i].type == a || tokens[i].type == b)
                return i;
        }
    }
    return 0;
}

int eval(uint32_t p, uint32_t q) {
    if (p > q) {
        /* Bad expression */
        assert(0);
    } else if (p == q) {
        /* Single token.
         * For now this token should be a number.
         * Return the value of the number.
         */
        switch (tokens[p].type) {
        case TK_DNUM:
            return atoi(tokens[p].str);
        case TK_XNUM:
            return atoi(tokens[p].str + 2); // 除去前面的0x
        default:
            return 0;
        }
    } else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses.
         */
        return eval(p + 1, q - 1);
    } else {
        uint32_t op;
        op = main_sign(p, q, '*', '/');
        op = main_sign(p, q, '+', '-'); // 后加减先乘除，若有加减则会覆盖
        if(op == 0) assert(0);

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
            if (val2 == 0)
                assert(0);
            return val1 / val2;
        default:
            assert(0);
        }
    }
}

word_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    eval(0, ptoken - 1);
    return 0;
}
