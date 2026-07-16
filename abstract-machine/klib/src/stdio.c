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
    char buf[1024];
    va_list args;
    va_start(args, fmt);

    int count = vsnprintf(buf, sizeof(buf), fmt, args);
    for (int i = 0; i <= count; i++) {
        putch(buf[i]);
    }
    return count;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    return vsnprintf(out, (size_t)-1, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    return vsnprintf(out, (size_t)-1, fmt, args);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    return vsnprintf(out, n, fmt, args);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    int count = 0;
    char *str;
    const char *buf = fmt;
    
    size_t max_n = n - 1;

    while (*buf != '\0' && count < max_n) {
        if (*buf == '%') {
            switch (*(buf + 1)) {
            case 'd': {
                int num = va_arg(ap, int);
                itoa(num, out, &count);
                break;
            }
            case 's': {
                str = va_arg(ap, char *);
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
                break;
            }
            buf++; // 跳过 '%'
        } else {
            out[count++] = *buf;
        }
        buf++;
    }

    out[count] = '\0'; // 添加字符串结束符
    return count;
}

#endif
