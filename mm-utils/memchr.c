/* [ memchr.c ] - scan memory for a character
 * -----------------------------------------------------------------------------
 * Copyright (c) 2010, Matteo Bertozzi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL MATTEO BERTOZZI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */

#include <stddef.h>
#include <stdint.h>

#define __mask16(x)      (((x) << 8) + (x))
#define __mask32(x)      (((x) << 24) + ((x) << 16) + __mask16(x))
#define __mask48(x)      (((x) << 40) + ((x) << 32) + __mask32(x))
#define __mask64(x)      (((x) << 56) + ((x) << 48) + __mask48(x))

void *memchr8 (const void *s, uint8_t c, size_t n) {
    register const uint8_t *p = (const uint8_t *)s;

    while (n--) {
        if (*p == c)
            return((void *)p);
        p++;
    }

    return(NULL);
}

void *memchr16 (const void *s, uint8_t c, size_t n) {
    register const uint16_t *ip = (const uint16_t *)s;
    register const uint8_t *p;
    register uint16_t mask;
    register uint16_t x;

    mask = __mask16((uint16_t)c);
    while (n >= sizeof(uint16_t)) {
        x = *ip ^ mask;
        if ((x - 0x0101U) & ~x & 0x8080U)
            break;

        n -= sizeof(uint16_t);
        ip++;
    }

    p = (const uint8_t *)ip;
    while (n--) {
        if (*p == c)
            return((void *)p);
        p++;
    }

    return(NULL);
}

void *memchr32 (const void *s, uint8_t c, size_t n) {
    register const uint32_t *ip = (const uint32_t *)s;
    register const uint8_t *p;
    register uint32_t mask;
    register uint32_t x;

    mask = __mask32((uint32_t)c);
    while (n >= sizeof(uint32_t)) {
        x = *ip ^ mask;
        if ((x - 0x01010101U) & ~x & 0x80808080U)
            break;

        n -= sizeof(uint32_t);
        ip++;
    }

    p = (const uint8_t *)ip;
    while (n--) {
        if (*p == c)
            return((void *)p);
        p++;
    }

    return(NULL);
}

void *memchr64 (const void *s, uint8_t c, size_t n) {
    register const uint64_t *ip = (const uint64_t *)s;
    register const uint8_t *p;
    register uint64_t mask;
    register uint64_t x;

    mask = __mask64((uint64_t)c);
    while (n >= sizeof(uint64_t)) {
        x = *ip ^ mask;
        if ((x - 0x0101010101010101ULL) & ~(x) & 0x8080808080808080ULL)
            break;

        n -= sizeof(uint64_t);
        ip++;
    }

    p = (const uint8_t *)ip;
    while (n--) {
        if (*p == c)
            return((void *)p);
        p++;
    }

    return(NULL);
}

void *memchr (const void *s, uint8_t c, size_t n) {
    switch ((n < sizeof(unsigned long)) ? n : sizeof(unsigned long)) {
        case 8: return(memchr64(s, c, n));
        case 4: return(memchr32(s, c, n));
        case 2: return(memchr16(s, c, n));
    }
    return(memchr8(s, c, n));
}


void *memrchr8 (const void *s, uint8_t c, size_t n) {
    register const uint8_t *p = (const uint8_t *)s;

    while (n--) {
        if (p[n] == c)
            return((void *)(p + n));
    }

    return(NULL);
}

void *memrchr16 (const void *s, uint8_t c, size_t n) {
    register const uint16_t *ip;
    register const uint8_t *p;
    register uint16_t mask;
    register uint16_t x;

    ip = (const uint16_t *)((const uint8_t *)s + n - 1 - sizeof(uint16_t));
    mask = __mask16((uint16_t)c);
    while (n >= sizeof(uint16_t)) {
        x = *ip ^ mask;
        if ((x - 0x0101U) & ~x & 0x8080U)
            break;

        n -= sizeof(uint16_t);
        ip--;
    }

    p = (const uint8_t *)ip;
    while (n--) {
        if (p[n] == c)
            return((void *)(p + n));
    }

    return(NULL);
}

void *memrchr32 (const void *s, uint8_t c, size_t n) {
    register const uint32_t *ip;
    register const uint8_t *p;
    register uint32_t mask;
    register uint32_t x;

    ip = (const uint32_t *)((const uint8_t *)s + n - 1 - sizeof(uint32_t));
    mask = __mask32((uint32_t)c);
    while (n >= sizeof(uint32_t)) {
        x = *ip ^ mask;
        if ((x - 0x01010101U) & ~x & 0x80808080U)
            break;

        n -= sizeof(uint32_t);
        ip--;
    }

    p = (const uint8_t *)ip;
    while (n--) {
        if (p[n] == c)
            return((void *)(p + n));
    }

    return(NULL);
}

void *memrchr64 (const void *s, uint8_t c, size_t n) {
    register const uint64_t *ip;
    register const uint8_t *p;
    register uint64_t mask;
    register uint64_t x;

    ip = (const uint64_t *)((const uint8_t *)s + n - 1 - sizeof(uint64_t));
    mask = __mask64((uint64_t)c);
    while (n >= sizeof(uint64_t)) {
        x = *ip ^ mask;
        if ((x - 0x0101010101010101ULL) & ~(x) & 0x8080808080808080ULL)
            break;

        n -= sizeof(uint64_t);
        ip--;
    }

    p = (const uint8_t *)ip;
    while (n--) {
        if (p[n] == c)
            return((void *)(p + n));
    }

    return(NULL);
}

void *memrchr (const void *s, uint8_t c, size_t n) {
    switch ((n < sizeof(unsigned long)) ? n : sizeof(unsigned long)) {
        case 8: return(memrchr64(s, c, n));
        case 4: return(memrchr32(s, c, n));
        case 2: return(memrchr16(s, c, n));
    }
    return(memrchr8(s, c, n));
}
