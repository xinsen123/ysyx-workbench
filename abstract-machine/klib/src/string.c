#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
    // panic("Not implemented");
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
    // panic("Not implemented");
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
    printf("Used\n");
}

int strcmp(const char *s1, const char *s2) {
    // panic("Not implemented");
    int i = 0, count = 0;
    while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0')
        i++;
    return s1[i] == s2[i] ? 0 : s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
    panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
    // panic("Not implemented");
    char *buf = (char *)s;
    int i;
    for (i = 0; i < n; i++) {
        buf[i] = c;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
    panic("Not implemented");
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
