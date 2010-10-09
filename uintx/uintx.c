/* [ uintx.h ] - Unsigned Int of specified bit length.
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

#include <string.h>
#include <stdio.h>

#include "uintx.h"

#define __BITMASK(n)            (1 << ((n) & 0x7))
#define __BITVMASK(n, v)        ((!!(v)) << ((n) & 0x7))
#define __BIT(map, n)           (*(((unsigned char *)(map)) + ((n) >> 3)))

#define __bit_set(map, n)       do { __BIT(map, n) |= __BITMASK(n); } while (0)
#define __bit_clear(map, n)     do { __BIT(map, n) &= ~__BITMASK(n); } while (0)
#define __bit_test(map, n)      (!!(__BIT(map, n) & __BITMASK(n)))

#define __bit_vset(map, n, v)                   \
    do {                                        \
        __bit_clear(map, n);                    \
        __BIT(map, n) |= __BITVMASK(n, v);      \
    } while(0)

#define __uintx_nbytes(bits)    ((((bits) + 7) & (-8)) >> 3)

/** Initialize uintx (equivalent to call uintx_from_u64(x, n, 0U) */
void uintx_init (uint8_t *x, size_t n) {
    memset(x, 0, __uintx_nbytes(n));
}

/** Initialize uintx at specified uint64_t value */
void uintx_from_u64 (uint8_t *x, size_t n, uint64_t value) {
    size_t bit = 0;

    memset(x, 0, __uintx_nbytes(n));
    while (value != 0) {
        __bit_vset(x, bit, value & 0x1);
        value >>= 1;
        bit++;
    }
}

/** Return true (1) if uintx is zero */
int uintx_is_zero (uint8_t *x, size_t n) {
    uint8_t *pe = x + __uintx_nbytes(n) - 1;
    uint8_t *p = x;

    for (; p < pe; ++p) {
        if (*p != 0)
            return(0);
    }

    return(1);
}

/** Increment unintx, return 1 on overflow. */
int uintx_inc (uint8_t *x, size_t n) {
    uint8_t *pe = x + __uintx_nbytes(n) - 1;
    uint8_t *p = x;

    while (++(*p) == 0 && p < pe)
        p++;

    return(p == pe && *p == 0);
}

/** Decrement unintx, return 1 on overflow. */
int uintx_dec (uint8_t *x, size_t n) {
    uint8_t *pe = x + __uintx_nbytes(n) - 1;
    uint8_t *p = x;

    while ((*p)-- == 0 && p < pe)
        p++;

    return(p == pe && *p == 0);
}

/** Add b to a (a must be larger enough to contains a + b) */
void uintx_add (uint8_t *a, size_t an, const uint8_t *b, size_t bn) {
    unsigned int carry = 0U;
    unsigned int d;
    size_t i = 0U;

    while (i < an && i < bn) {
        d = __bit_test(a, i) + __bit_test(b, i) + carry;
        __bit_vset(a, i, d & 0x1);
        carry = d >> 1;
        i++;
    }

    while (i < an) {
        d = __bit_test(a, i) + carry;
        __bit_vset(a, i, d & 0x1);
        carry = d >> 1;
        i++;
    }

    while (i < bn) {
        d = __bit_test(b, i) + carry;
        __bit_vset(a, i, d & 0x1);
        carry = d >> 1;
        i++;
    }

    if (carry != 0)
        __bit_set(a, i);
}

/** Add Specified uint64_t value to x */
void uintx_add64 (uint8_t *x, size_t n, uint64_t value) {
    uint8_t x64[8];
    uintx_from_u64(x64, 64U, value);
    uintx_add(x, n, x64, 64U);
}

void uintx_and (uint8_t *a, size_t an, const uint8_t *b, size_t bn) {
    unsigned int v;
    size_t i;

    for (i = 0U; i < an && i < bn; ++i) {
        v = __bit_test(a, i) & __bit_test(b, i);
        __bit_vset(a, i, v);
    }
}

void uintx_and64 (uint8_t *x, size_t n, uint64_t value) {
    uint8_t x64[8];
    uintx_from_u64(x64, 64U, value);
    uintx_and(x, n, x64, 64U);
}

void uintx_xor (uint8_t *a, size_t an, const uint8_t *b, size_t bn) {
    unsigned int v;
    size_t i;

    for (i = 0U; i < an && i < bn; ++i) {
        v = __bit_test(a, i) ^ __bit_test(b, i);
        __bit_vset(a, i, v);
    }
}

void uintx_xor64 (uint8_t *x, size_t n, uint64_t value) {
    uint8_t x64[8];
    uintx_from_u64(x64, 64U, value);
    uintx_xor(x, n, x64, 64U);
}

void uintx_or (uint8_t *a, size_t an, const uint8_t *b, size_t bn) {
    unsigned int v;
    size_t i;

    for (i = 0U; i < an && i < bn; ++i) {
        v = __bit_test(a, i) | __bit_test(b, i);
        __bit_vset(a, i, v);
    }
}

void uintx_or64 (uint8_t *x, size_t n, uint64_t value) {
    uint8_t x64[8];
    uintx_from_u64(x64, 64U, value);
    uintx_or(x, n, x64, 64U);
}

/** Compare a and b and returns:
 *       0   a and b are equals
 *       1   a is greater than b
 *      -1   a is less than b
 */
int uintx_compare (const uint8_t *a, size_t an, const uint8_t *b, size_t bn) {
    unsigned int v;
    const uint8_t *p2;
    const uint8_t *p1;

    if (an > bn) {
        p1 = a + __uintx_nbytes(an) - 1;
        while ((an - bn) >= 8U) {
            if (*p1 != 0)
                return(1);
            an -= 8U;
            p1--;
        }

        while (an > bn) {
            an--;
            if (__bit_test(a, an))
                return(1);
        }
    }

    if (bn > an) {
        p2 = b + __uintx_nbytes(bn) - 1;
        while ((bn - an) >= 8U) {
            if (*p2 != 0)
                return(-1);
            bn -= 8U;
            p2--;
        }

        while (bn > an) {
            bn--;
            if (__bit_test(b, bn))
                return(-1);
        }
    }

    p1 = a + __uintx_nbytes(an) - 1;
    p2 = b + __uintx_nbytes(an) - 1;
    while (an >= 8U) {
        if (*p1 != *p2)
            break;
        an -= 8U;
        p1--;
        p2--;
    }

    while (an > 0) {
        an--;
        if ((v = __bit_test(a, an)) != __bit_test(b, an))
            return(v ? 1 : -1);
    }

    return(0);
}

/** Compare specified uint64_t value with x */
int uintx_compare64 (const uint8_t *x, size_t n, uint64_t value) {
    uint8_t x64[8];
    uintx_from_u64(x64, 64U, value);
    return(uintx_compare(x, n, x64, 64U));
}

/** Print specified uintx in binary form */
void uintx_print (const uint8_t *x, size_t n) {
    size_t nbytes = __uintx_nbytes(n);
    const uint8_t *p;

    for (p = x; nbytes--; ++p) {
        printf("%d%d%d%d%d%d%d%d", __bit_test(p, 0),
                                   __bit_test(p, 1),
                                   __bit_test(p, 2),
                                   __bit_test(p, 3),
                                   __bit_test(p, 4),
                                   __bit_test(p, 5),
                                   __bit_test(p, 6),
                                   __bit_test(p, 7));
    }
}

