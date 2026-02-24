#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void itoa(int num, char *dst, int *count) {
    if (num != 0)
        itoa(num / 10, dst + 1, count);
    *dst = (num % 10) + '0';
    count++;
}

int printf(const char *fmt, ...) {
    panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    // panic("Not implemented");
    va_list args;
    va_start(args, fmt);
    int count = 0;
    char *str;

    const char *buf = fmt;
    while (*buf != '\0') {
        if (*buf == '%') {
            switch (*(buf + 1)) {
            case 'd':
                itoa(va_arg(args, int), out + count, &count);
                break;
            case 's':
                str = va_arg(args, char *);
                while (*str != '\0') {
                    *(out + count) = *str;
                    count++;
                    str++;
                }
                break;
            case '%':
                *(out + count) = '%';
                break;
            }
            buf+=2;
        } else {
            *(out + count) = *buf;
            count++;
        }
        buf++;
    }
    return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
