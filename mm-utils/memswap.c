/* [ memswap.c ] - Memory Swap
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

