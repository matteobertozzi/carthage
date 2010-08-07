/* [ memcmp.c ] - Memory Compare
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

#define __cmp8_step(m1, m2)                                                 \
    do {                                                                    \
        if (m1[0] != m2[0]) return(m1[0] - m2[0]);                          \
    } while (0)

#define __cmp16_step(m1, m2)                                                \
    do {                                                                    \
        if (m1[0] != m2[0]) return(m1[0] - m2[0]);                          \
        if (m1[1] != m2[1]) return(m1[1] - m2[1]);                          \
    } while (0)

#define __cmp32_step(m1, m2)                                                \
    do {                                                                    \
        if (m1[0] != m2[0]) return(m1[0] - m2[0]);                          \
        if (m1[1] != m2[1]) return(m1[1] - m2[1]);                          \
        if (m1[2] != m2[2]) return(m1[2] - m2[2]);                          \
        if (m1[3] != m2[3]) return(m1[3] - m2[3]);                          \
    } while (0)

#define __cmp64_step(m1, m2)                                                \
    do {                                                                    \
        if (m1[0] != m2[0]) return(m1[0] - m2[0]);                          \
        if (m1[1] != m2[1]) return(m1[1] - m2[1]);                          \
        if (m1[2] != m2[2]) return(m1[2] - m2[2]);                          \
        if (m1[3] != m2[3]) return(m1[3] - m2[3]);                          \
        if (m1[4] != m2[4]) return(m1[4] - m2[4]);                          \
        if (m1[5] != m2[5]) return(m1[5] - m2[5]);                          \
        if (m1[6] != m2[6]) return(m1[6] - m2[6]);                          \
        if (m1[7] != m2[7]) return(m1[7] - m2[7]);                          \
    } while (0)

#define __cmp_generic_step(m1, m2, n)                                       \
    do {                                                                    \
        unsigned long i;                                                    \
        for (i = 0; i < n; ++i) {                                           \
            if (m1[i] != m2[i])                                             \
                return(m1[i] - m2[i]);                                      \
        }                                                                   \
    } while (0)

int memcmp (const void *m1, const void *m2, size_t n) {
    const unsigned long *im1 = (const unsigned long *)m1;
    const unsigned long *im2 = (const unsigned long *)m2;
    const unsigned char *cm1;
    const unsigned char *cm2;

    /* Fast compare with n >= sizeof(unsigned long) */
    for (; n >= sizeof(unsigned long); n -= sizeof(unsigned long)) {
        if (*im1 != *im2) {
            cm1 = (const unsigned char *)im1;
            cm2 = (const unsigned char *)im2;

            switch (sizeof(unsigned long)) {
                case 1:
                    __cmp8_step(cm1, cm2);
                case 2:
                    __cmp16_step(cm1, cm2);
                    break;
                case 4:
                    __cmp32_step(cm1, cm2);
                    break;
                case 8:
                    __cmp64_step(cm1, cm2);
                    break;
                default:
                    __cmp_generic_step(cm1, cm2, sizeof(unsigned long));
                    break;
            }
        }

        im1++;
        im2++;
    }

    /* Compare the rest in a traditional way */
    cm1 = (const unsigned char *)im1;
    cm2 = (const unsigned char *)im2;
    for (; n--; cm1++, cm2++) {
        if (*cm1 != *cm2)
            return(*cm1 - *cm2);
    }

    return(0);
}

int memcmp8 (const void *m1, const void *m2, size_t n) {
    const uint8_t *cm1 = (const uint8_t *)m1;
    const uint8_t *cm2 = (const uint8_t *)m2;

    for (; n--; cm1++, cm2++) {
        if (*cm1 != *cm2)
            return(*cm1 - *cm2);
    }

    return(0);
}

int memcmp16 (const void *m1, const void *m2, size_t n) {
    const uint16_t *im1 = (const uint16_t *)m1;
    const uint16_t *im2 = (const uint16_t *)m2;
    const uint8_t *cm1;
    const uint8_t *cm2;

    for (; n >= sizeof(uint16_t); n -= sizeof(uint16_t)) {
        if (*im1 != *im2) {
            cm1 = (const uint8_t *)im1;
            cm2 = (const uint8_t *)im2;
            __cmp16_step(cm1, cm2);
        }
        im1++;
        im2++;
    }

    return(0);
}

int memcmp32 (const void *m1, const void *m2, size_t n) {
    const uint32_t *im1 = (const uint32_t *)m1;
    const uint32_t *im2 = (const uint32_t *)m2;
    const uint8_t *cm1;
    const uint8_t *cm2;

    for (; n >= sizeof(uint32_t); n -= sizeof(uint32_t)) {
        if (*im1 != *im2) {
            cm1 = (const uint8_t *)im1;
            cm2 = (const uint8_t *)im2;
            __cmp32_step(cm1, cm2);
        }
        im1++;
        im2++;
    }

    return(0);
}

int memcmp64 (const void *m1, const void *m2, size_t n) {
    const uint64_t *im1 = (const uint64_t *)m1;
    const uint64_t *im2 = (const uint64_t *)m2;
    const uint8_t *cm1;
    const uint8_t *cm2;

    for (; n >= sizeof(uint64_t); n -= sizeof(uint64_t)) {
        if (*im1 != *im2) {
            cm1 = (const uint8_t *)im1;
            cm2 = (const uint8_t *)im2;
            __cmp64_step(cm1, cm2);
        }
        im1++;
        im2++;
    }

    return(0);
}
