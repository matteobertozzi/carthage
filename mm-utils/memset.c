/* [ memset.c ] - Memory Fill (Set/Zero)
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

#include <stdint.h>
#include <stddef.h>

#define __memset(dest, c, pattern, n)                                       \
    do {                                                                    \
        unsigned long *ip = (unsigned long *)dest;                          \
        unsigned char *cp;                                                  \
                                                                            \
        /* Fast set with n >= sizeof(unsigned long) */                      \
        for (; n >= sizeof(unsigned long); n -= sizeof(unsigned long))      \
            *ip++ = pattern;                                                \
                                                                            \
        /* Set the rest of the buffer */                                    \
        cp = (unsigned char *)ip;                                           \
        while (n--)                                                         \
            *cp++ = c;                                                      \
    } while (0)


#define __memset_sized(type, dest, pattern, n)                              \
    do {                                                                    \
        type *p = (type *)dest;                                             \
                                                                            \
        for (; n >= sizeof(type); n -= sizeof(type))                        \
            *p++ = pattern;                                                 \
    } while (0)

/*
 * Mem-Set
 */
void *memset (void *dest, int c, size_t n) {
    unsigned long pattern = (unsigned long)c;

    switch (sizeof(unsigned long)) {
        case 4:
            pattern |= ((uint32_t)c) <<  8;
            pattern |= ((uint32_t)c) << 16;
            pattern |= ((uint32_t)c) << 24;
            __memset(dest, c, pattern, n);
            break;
        case 8:
            pattern |= ((uint64_t)c) <<  8;
            pattern |= ((uint64_t)c) << 16;
            pattern |= ((uint64_t)c) << 24;
            pattern |= ((uint64_t)c) << 32;
            pattern |= ((uint64_t)c) << 40;
            pattern |= ((uint64_t)c) << 48;
            pattern |= ((uint64_t)c) << 56;
            __memset(dest, c, pattern, n);
            break;
        default:
            __memset_sized(uint8_t, dest, c, n);
            break;
    }

    return(dest);
}

void *memset8 (void *dest, uint8_t c, size_t n) {
    __memset_sized(uint8_t, dest, c, n);
    return(dest);
}

void *memset16 (void *dest, uint8_t c, size_t n) {
    uint16_t pattern = (uint16_t)c;
    pattern |= ((uint32_t)c) <<  8;
    __memset_sized(uint16_t, dest, c, n);
    return(dest);
}

void *memset32 (void *dest, uint8_t c, size_t n) {
    uint32_t pattern = (uint32_t)c;
    pattern |= ((uint32_t)c) <<  8;
    pattern |= ((uint32_t)c) << 16;
    pattern |= ((uint32_t)c) << 24;
    __memset_sized(uint32_t, dest, pattern, n);
    return(dest);
}

void *memset64 (void *dest, uint8_t c, size_t n) {
    uint64_t pattern = (uint64_t)c;
    pattern |= ((uint64_t)c) <<  8;
    pattern |= ((uint64_t)c) << 16;
    pattern |= ((uint64_t)c) << 24;
    pattern |= ((uint64_t)c) << 32;
    pattern |= ((uint64_t)c) << 40;
    pattern |= ((uint64_t)c) << 48;
    pattern |= ((uint64_t)c) << 56;
    __memset_sized(uint64_t, dest, pattern, n);
    return(dest);
}

/*
 * Mem-Zero
 */
void *memzero (void *dest, size_t n) {
    switch (sizeof(unsigned long)) {
        case 2: {
            uint16_t z16 = 0U;
            __memset_sized(uint16_t, dest, z16, n);
        } case 4: {
            uint32_t z32 = 0U;
            __memset_sized(uint32_t, dest, z32, n);
            break;
        } case 8: {
            uint64_t z64 = 0U;
            __memset_sized(uint64_t, dest, z64, n);
            break;
        } default: {
            uint8_t z8 = 0U;
            __memset_sized(uint8_t, dest, z8, n);
            break;
        }
    }
    return(dest);
}

void *memzero8 (void *dest, size_t n) {
    uint8_t z8 = 0U;
    __memset_sized(uint8_t, dest, z8, n);
    return(dest);
}

void *memzero16 (void *dest, size_t n) {
    uint16_t z16 = 0U;
    __memset_sized(uint16_t, dest, z16, n);
    return(dest);
}

void *memzero32 (void *dest, size_t n) {
    uint32_t z32 = 0U;
    __memset_sized(uint32_t, dest, z32, n);
    return(dest);
}

void *memzero64 (void *dest, size_t n) {
    uint64_t z64 = 0U;
    __memset_sized(uint64_t, dest, z64, n);
    return(dest);
}
