#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void itoa(int num, char *dst, int *count) {
    // 处理 0 的特殊情况
    if (num == 0) {
        dst[(*count)++] = '0';
        return;
    }

    // 处理负数
    unsigned int unum;
    if (num < 0) {
        dst[(*count)++] = '-';
        unum = (unsigned int)(-(num + 1)) + 1; // 安全处理 INT_MIN
    } else {
        unum = (unsigned int)num;
    }

    // 递归处理高位
    if (unum / 10 != 0) {
        itoa(unum / 10, dst, count); // 注意：这里传入的是 unsigned int
    }

    // 添加当前位
    dst[(*count)++] = (unum % 10) + '0';
}

int printf(const char *fmt, ...) {
    panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = 0;
    char *str;

    const char *buf = fmt;
    while (*buf != '\0') {
        if (*buf == '%') {
            switch (*(buf + 1)) {
            case 'd': {
                int num = va_arg(args, int);
                itoa(num, out, &count);
                break;
            }
            case 's': {
                str = va_arg(args, char *);
                if (str) {
                    while (*str != '\0') {
                        out[count++] = *str++;
                    }
                }
                break;
            }
            case '%': {
                out[count++] = '%';
                break;
            }
            default:
                // 未知格式，原样输出
                out[count++] = '%';
                out[count++] = *buf;
                buf++;
                break;
            }
            buf++; // 跳过 '%'
        } else {
            out[count++] = *buf;
        }
        buf++;
    }

    out[count] = '\0'; // 添加字符串结束符
    va_end(args);
    return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
