#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    size_t i = 0;
    while(*s != '\0') i++;
    return i;
}

char *strcpy(char *dst, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    size_t i = 0;
    while(src[i] !='\0' && i <= n){
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return dst;
}

char *strcat(char *dst, const char *src) {
    int i = 0;
    const char *buf = src;
    while (dst[i] != '\0')
        i++;
    while (*buf != '\0') {
        dst[i] = *buf;
        i++;
        buf++;
    }
    dst[i] = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0')
        i++;
    return s1[i] == s2[i] ? 0 : s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i = 0;
    while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0' && i <= n)
        i++;
    return s1[i] == s2[i] ? 0 : s1[i] - s2[i];
}

void *memset(void *s, int c, size_t n) {
    char *buf = (char *)s;
    int i;
    for (i = 0; i < n; i++) {
        buf[i] = c;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    char *d = (char *)dst;
    char *s = (char *)src;

    if(d > s){
        for(size_t i = n; i > 0; i--) d[i] = s[i];
    } else if (s < d) {
        for(size_t i = 0; i < n; i++) d[i] = s[i];
    }

    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
    char *o = (char *)out;
    char *i = (char *)in;

    while (n--) {
        o[n] = i[n];
    }

    return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    // panic("Not implemented");
    const char *p1 = (const char *)s1;
    const char *p2 = (const char *)s2;
    int i;
    for (i = 0; i < n; i++) {
        if (p1[i] != p2[i])
            return *p1 - *p2;
    }
    return 0;
}

#endif
