/* [ bitarray.c ] - BitArray (Bitmap) Utils
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

#include "bitarray.h"

static ssize_t __bit_find_first (unsigned char *map,
                                 size_t size,
                                 size_t offset,
                                 int value)
{
    unsigned char *p = __BIT(map, offset);
    char pattern[2] = { 0xff, 0x00 };

    if ((offset = (offset & 0x7))) {
        for (; offset < 8U; offset++) {
            if (!!bit_test(p, offset) == value)
                return(((p - ((unsigned char *)map)) << 3) + offset);
        }
        p++;
    }

    size = (size << 3) - ((p - ((unsigned char *)map)) << 3);
    size = (size >> 3) + ((size & 0x7) > 0);

    if (size > 0) {
        while (*p++ == pattern[value]) {
            if (--size == 0)
                return(-1);
        }

        --p;
        for (offset = 0; offset < 8U; ++offset) {
            if (!!bit_test(p, offset) == value)
                return(((p - map) << 3) + offset);
        }
    }

    return(-1);
}

int bitarray_set (bitarray_t *array, size_t n) {
    int old_state;

    old_state = bit_test(array->map, n);
    bit_set(array->map, n);

    return(old_state);
}

int bitarray_clear (bitarray_t *array, size_t n) {
    int old_state;

    old_state = bit_test(array->map, n);
    bit_clear(array->map, n);

    return(old_state);
}

int bitarray_test (bitarray_t *array, size_t n) {
    return(bit_test(array->map, n));
}

void bitarray_set_bits (bitarray_t *array, size_t start, size_t count) {
    unsigned char *p;

    p = __BIT(array->map, start);
    while (count >= 8U) {
        *p |= __BITMASK(start++);
        *p |= __BITMASK(start++);
        *p |= __BITMASK(start++);
        *p |= __BITMASK(start++);
        *p |= __BITMASK(start++);
        *p |= __BITMASK(start++);
        *p |= __BITMASK(start++);
        *p |= __BITMASK(start++);
        count -= 8U;
        p++;
    }

    while (count--) {
        bit_set(array->map, start);
        start++;
    }
}

void bitarray_clear_bits (bitarray_t *array, size_t start, size_t count) {
    unsigned char *p;

    p = __BIT(array->map, start);
    while (count >= 8U) {
        *p &= ~__BITMASK(start++);
        *p &= ~__BITMASK(start++);
        *p &= ~__BITMASK(start++);
        *p &= ~__BITMASK(start++);
        *p &= ~__BITMASK(start++);
        *p &= ~__BITMASK(start++);
        *p &= ~__BITMASK(start++);
        *p &= ~__BITMASK(start++);
        count -= 8U;
        p++;
    }

    while (count--) {
        bit_clear(array->map, start);
        start++;
    }
}

ssize_t bitarray_find_set (bitarray_t *array, size_t offset) {
    return(__bit_find_first(array->map, array->size, offset, 1));
}

ssize_t bitarray_find_zero (bitarray_t *array, size_t offset) {
    return(__bit_find_first(array->map, array->size, offset, 0));
}

