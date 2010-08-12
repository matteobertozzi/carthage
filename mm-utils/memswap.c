#include <stdint.h>
#include <stddef.h>

#define __memswap_sized(type, a, b, n)                                      \
    do {                                                                    \
        type *ia = (type *)a;                                               \
        type *ib = (type *)b;                                               \
        type it;                                                            \
                                                                            \
        for (; n >= sizeof(type); n -= sizeof(type)) {                      \
            it = *ia;                                                       \
            *ia++ = *ib;                                                    \
            *ib++ = it;                                                     \
        }                                                                   \
    } while (0)

void memswap (void *a, void *b, size_t n) {
    unsigned long *ia = (unsigned long *)a;
    unsigned long *ib = (unsigned long *)b;
    unsigned char *ca;
    unsigned char *cb;
    unsigned long it;
    unsigned char ct;

    /* Fast copy while n >= sizeof(unsigned long) */
    for (; n >= sizeof(unsigned long); n -= sizeof(unsigned long)) {
        it = *ia;
        *ia++ = *ib;
        *ib++ = it;
    }

    /* Copy the rest of the data */
    ca = (unsigned char *)ia;
    cb = (unsigned char *)ib;
    while (n--) {
        ct = *ca;
        *ca++ = *cb;
        *cb++ = ct;
    }
}

void memswap8 (void *a, void *b, size_t n) {
    __memswap_sized(uint8_t, a, b, n);
}

void memswap16 (void *a, void *b, size_t n) {
    __memswap_sized(uint16_t, a, b, n);
}

void memswap32 (void *a, void *b, size_t n) {
    __memswap_sized(uint32_t, a, b, n);
}

void memswap64 (void *a, void *b, size_t n) {
    __memswap_sized(uint64_t, a, b, n);
}

