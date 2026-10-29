#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// 返回值沿用之前的约定: "实际写入的长度"(不是 C99 的"本该写入的长度").
// 缓冲区始终以 '\0' 结尾(snprintf(dst, 0, ...) 除外).
static void out_ch(char *out, size_t max_n, int *count, char ch) {
    if ((size_t)*count < max_n) {
        out[*count] = ch;
        (*count)++;
    }
}

static void out_str(char *out, size_t max_n, int *count, const char *s, int len) {
    for (int i = 0; i < len; i++) {
        out_ch(out, max_n, count, s[i]);
    }
}

// 把无符号数按 base 转成字符串, 返回长度. tmp 至少 34 字节(64 位二进制).
static int utoa(unsigned long v, unsigned base, bool upper, char *tmp) {
    static const char lo[] = "0123456789abcdef";
    static const char up[] = "0123456789ABCDEF";
    const char *digits = upper ? up : lo;

    if (v == 0) {
        tmp[0] = '0';
        return 1;
    }

    int len = 0;
    while (v != 0) {
        tmp[len++] = digits[v % base];
        v /= base;
    }
    for (int i = 0; i < len / 2; i++) {
        char t = tmp[i];
        tmp[i] = tmp[len - 1 - i];
        tmp[len - 1 - i] = t;
    }
    return len;
}

// 按宽度输出: 前缀(符号/0x) + 填充 + 主体; left 时填充在主体之后.
static void out_padded(char *out, size_t max_n, int *count,
                       const char *prefix, int plen,
                       const char *body, int blen,
                       int width, bool left, char pad) {
    int total = plen + blen;
    int nfill = width > total ? width - total : 0;

    out_str(out, max_n, count, prefix, plen);
    if (!left) {
        for (int i = 0; i < nfill; i++)
            out_ch(out, max_n, count, pad);
    }
    out_str(out, max_n, count, body, blen);
    if (left) {
        for (int i = 0; i < nfill; i++)
            out_ch(out, max_n, count, ' ');
    }
}

static int strnlen_(const char *s, int max) {
    int n = 0;
    while (n < max && s[n] != '\0')
        n++;
    return n;
}

int printf(const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);

    int count = vsnprintf(buf, sizeof(buf), fmt, args);
    for (int i = 0; i < count; i++) {
        putch(buf[i]);
    }
    va_end(args);
    return count;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    return vsnprintf(out, (size_t)-1, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = vsnprintf(out, (size_t)-1, fmt, args);
    va_end(args);
    return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = vsnprintf(out, n, fmt, args);
    va_end(args);
    return count;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    int count = 0;
    size_t max_n = n == 0 ? 0 : n - 1;
    const char *f = fmt;

    while (*f != '\0') {
        if (*f != '%') {
            out_ch(out, max_n, &count, *f++);
            continue;
        }

        f++; // 跳过 '%'
        bool left = false, zero = false;
        for (;; f++) {
            if (*f == '-') {
                left = true;
            } else if (*f == '0') {
                zero = true;
            } else if (*f == '+' || *f == ' ' || *f == '#') {
                // 暂时忽略符号/前缀类标志: PA 中没有用到
            } else {
                break;
            }
        }

        int width = 0;
        while (*f >= '0' && *f <= '9') {
            width = width * 10 + (*f - '0');
            f++;
        }

        int prec = -1;
        if (*f == '.') {
            f++;
            prec = 0;
            while (*f >= '0' && *f <= '9') {
                prec = prec * 10 + (*f - '0');
                f++;
            }
        }

        // 长度修饰符: 只区分"按 int 取"和"按 long/size_t 取"
        enum { LEN_INT,
               LEN_LONG,
               LEN_SIZE } len = LEN_INT;
        if (*f == 'l') {
            len = LEN_LONG;
            f++;
            if (*f == 'l')
                f++;
        } else if (*f == 'z') {
            len = LEN_SIZE;
            f++;
        } else if (*f == 'h') {
            f++;
            if (*f == 'h')
                f++;
        }

        char conv = *f;
        if (conv == '\0')
            break; // 格式串以 '%' 结尾: 直接结束
        f++;

        char tmp[34];
        char prefix[4];
        int plen = 0;

        switch (conv) {
        case 'd':
        case 'i': {
            long v;
            if (len == LEN_SIZE)
                v = (long)va_arg(ap, size_t);
            else if (len == LEN_LONG)
                v = va_arg(ap, long);
            else
                v = va_arg(ap, int);

            unsigned long mag;
            if (v < 0) {
                prefix[plen++] = '-';
                mag = (unsigned long)(-(v + 1)) + 1; // 正确处理 INT_MIN/LONG_MIN
            } else {
                mag = (unsigned long)v;
            }
            int blen = utoa(mag, 10, false, tmp);
            out_padded(out, max_n, &count, prefix, plen, tmp, blen,
                       width, left, (zero && !left) ? '0' : ' ');
            break;
        }

        case 'u': {
            unsigned long v;
            if (len == LEN_SIZE)
                v = va_arg(ap, size_t);
            else if (len == LEN_LONG)
                v = va_arg(ap, unsigned long);
            else
                v = va_arg(ap, unsigned int);
            int blen = utoa(v, 10, false, tmp);
            out_padded(out, max_n, &count, prefix, 0, tmp, blen,
                       width, left, (zero && !left) ? '0' : ' ');
            break;
        }

        case 'x':
        case 'X': {
            unsigned long v;
            if (len == LEN_SIZE)
                v = va_arg(ap, size_t);
            else if (len == LEN_LONG)
                v = va_arg(ap, unsigned long);
            else
                v = va_arg(ap, unsigned int);
            int blen = utoa(v, 16, conv == 'X', tmp);
            out_padded(out, max_n, &count, prefix, 0, tmp, blen,
                       width, left, (zero && !left) ? '0' : ' ');
            break;
        }

        case 'p': {
            unsigned long v = (unsigned long)(uintptr_t)va_arg(ap, void *);
            prefix[plen++] = '0';
            prefix[plen++] = 'x';
            int blen = utoa(v, 16, false, tmp);
            out_padded(out, max_n, &count, prefix, plen, tmp, blen,
                       width, left, ' ');
            break;
        }

        case 'c': {
            char ch = (char)va_arg(ap, int);
            out_padded(out, max_n, &count, prefix, 0, &ch, 1,
                       width, left, ' ');
            break;
        }

        case 's': {
            const char *s = va_arg(ap, const char *);
            if (s == NULL)
                s = "(null)";
            int slen = prec >= 0 ? strnlen_(s, prec) : strnlen_(s, 1 << 30);
            out_padded(out, max_n, &count, prefix, 0, s, slen,
                       width, left, ' ');
            break;
        }

        case '%': {
            out_ch(out, max_n, &count, '%');
            break;
        }

        default:
            // 不认识的转换符: 原样输出 '%' 和该字符, 不消费后续参数
            out_ch(out, max_n, &count, '%');
            out_ch(out, max_n, &count, conv);
            break;
        }
    }

    if (n != 0)
        out[count] = '\0'; // 字符串结束符
    return count;
}

#endif
