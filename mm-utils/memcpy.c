/* [ memcpy.c ] - Memory Copy
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

#define __memcpy_forward(dst, src, n)                                       \
    do {                                                                    \
        const unsigned long *isrc = (const unsigned long *)src;             \
        unsigned long *idst = (unsigned long *)dst;                         \
        const unsigned char *csrc;                                          \
        unsigned char *cdst;                                                \
                                                                            \
        /* Fast copy while l >= sizeof(unsigned long) */                    \
        for (; n >= sizeof(unsigned long); n -= sizeof(unsigned long))      \
            *idst++ = *isrc++;                                              \
                                                                            \
        /* Copy the rest of the buffer */                                   \
        csrc = (const unsigned char *)isrc;                                 \
        cdst = (unsigned char *)idst;                                       \
        while (n--)                                                         \
            *cdst++ = *csrc++;                                              \
    } while (0)

#define __memcpy_forward_sized(type, dst, src, n)                           \
    do {                                                                    \
        const type *isrc = (const type *)src;                               \
        type *idst = (type *)dest;                                          \
                                                                            \
        for (; n >= sizeof(type); n -= sizeof(type))                        \
            *idst++ = *isrc++;                                              \
    } while (0)

#define __memcpy_backward(dst, src, n)                                      \
    do {                                                                    \
        const unsigned long *isrc = (const unsigned long *)(src + n);       \
        unsigned long *idst = (unsigned long *)(dst + n);                   \
        const unsigned char *csrc;                                          \
        unsigned char *cdst;                                                \
                                                                            \
        /* Fast copy while n >= sizeof(unsigned long) */                    \
        for (; n >= sizeof(unsigned long); n -= sizeof(unsigned long))      \
            *idst-- = *isrc--;                                              \
                                                                            \
        /* Copy the rest of the buffer */                                   \
        csrc = (const unsigned char *)isrc;                                 \
        cdst = (unsigned char *)idst;                                       \
        while (n--)                                                         \
            *cdst-- = *csrc--;                                              \
    } while (0)

#define __memcpy_backward_sized(type, dst, src, n)                          \
    do {                                                                    \
        const type *isrc = (const type *)(src + n);                         \
        type *idst = (type *)(dst + n);                                     \
                                                                            \
        for (; n > sizeof(type); n -= sizeof(type))                         \
            *idst-- = *isrc--;                                              \
    } while (0)

/*
 * Mem-Copy Forward
 */
void *memcpy (void *dest, const void *src, size_t n) {
    __memcpy_forward(dest, src, n);
    return(dest);
}

void *memcpy8 (void *dest, const void *src, size_t n) {
    __memcpy_forward_sized(uint8_t, dest, src, n);
    return(dest);
}

void *memcpy16 (void *dest, const void *src, size_t n) {
    __memcpy_forward_sized(uint16_t, dest, src, n);
    return(dest);
}

void *memcpy32 (void *dest, const void *src, size_t n) {
    __memcpy_forward_sized(uint32_t, dest, src, n);
    return(dest);
}

void *memcpy64 (void *dest, const void *src, size_t n) {
    __memcpy_forward_sized(uint64_t, dest, src, n);
    return(dest);
}

/*
 * Mem-Copy backward
 */
void *membcpy (void *dest, const void *src, size_t n) {
    __memcpy_backward(dest, src, n);
    return(dest);
}

void *membcpy8 (void *dest, const void *src, size_t n) {
    __memcpy_backward_sized(uint8_t, dest, src, n);
    return(dest);
}

void *membcpy16 (void *dest, const void *src, size_t n) {
    __memcpy_backward_sized(uint16_t, dest, src, n);
    return(dest);
}

void *membcpy32 (void *dest, const void *src, size_t n) {
    __memcpy_backward_sized(uint32_t, dest, src, n);
    return(dest);
}

void *membcpy64 (void *dest, const void *src, size_t n) {
    __memcpy_backward_sized(uint64_t, dest, src, n);
    return(dest);
}

/*
 * Mem-Move
 */
void *memmove (void *dest, const void *src, size_t n) {
    if (dest < src)
        __memcpy_forward(dest, src, n);
    else
        __memcpy_backward(dest, src, n);
    return(dest);
}

void *memmove8 (void *dest, const void *src, size_t n) {
    if (dest < src)
        __memcpy_forward_sized(uint8_t, dest, src, n);
    else
        __memcpy_backward_sized(uint8_t, dest, src, n);
    return(dest);
}

void *memmove16 (void *dest, const void *src, size_t n) {
    if (dest < src)
        __memcpy_forward_sized(uint16_t, dest, src, n);
    else
        __memcpy_backward_sized(uint16_t, dest, src, n);
    return(dest);
}

void *memmove32 (void *dest, const void *src, size_t n) {
    if (dest < src)
        __memcpy_forward_sized(uint32_t, dest, src, n);
    else
        __memcpy_backward_sized(uint32_t, dest, src, n);
    return(dest);
}

void *memmove64 (void *dest, const void *src, size_t n) {
    if (dest < src)
        __memcpy_forward_sized(uint64_t, dest, src, n);
    else
        __memcpy_forward_sized(uint64_t, dest, src, n);
    return(dest);
}

