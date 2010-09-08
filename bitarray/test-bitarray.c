/*
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

#include <stdio.h>

#include "bitarray.h"

static void __print_bitarray (bitarray_t *array) {
    unsigned char *p;
    size_t count;

    p = array->map;
    count = array->size;
    while (count--) {
        printf("%d%d%d%d%d%d%d%d", !!(*p & __BITMASK(0)),
                                   !!(*p & __BITMASK(1)),
                                   !!(*p & __BITMASK(2)),
                                   !!(*p & __BITMASK(3)),
                                   !!(*p & __BITMASK(4)),
                                   !!(*p & __BITMASK(5)),
                                   !!(*p & __BITMASK(6)),
                                   !!(*p & __BITMASK(7)));
    }
    printf("\n");
}

int main (int argc, char **argv) {
    unsigned char map[4];
    bitarray_t array;
    size_t i, n;

    array.map = map;
    array.size = sizeof(map);
    n = array.size << 3;

    bitarray_set_bits(&array, 0U, n);
    __print_bitarray(&array);

    printf("Bit Zero: ");
    for (i = 0U; i < n; ++i)
        printf(" %2ld", bitarray_find_zero(&array, i));
    printf("\n");

    printf("Bit Set:  ");
    for (i = 0U; i < n; ++i)
        printf(" %2ld", bitarray_find_set(&array, i));
    printf("\n");

    bitarray_clear_bits(&array, 0U, n);
    __print_bitarray(&array);

    printf("Bit Zero: ");
    for (i = 0U; i < n; ++i)
        printf(" %2ld", bitarray_find_zero(&array, i));
    printf("\n");

    printf("Bit Set:  ");
    for (i = 0U; i < n; ++i)
        printf(" %ld", bitarray_find_set(&array, i));
    printf("\n");

    for (i = 0U; i < n; i += 3U)
        bitarray_set(&array, i);
    __print_bitarray(&array);

    printf("Bit Zero: ");
    for (i = 0U; i < n; ++i)
        printf(" %2ld", bitarray_find_zero(&array, i));
    printf("\n");

    printf("Bit Set:  ");
    for (i = 0U; i < n; ++i)
        printf(" %2ld", bitarray_find_set(&array, i));
    printf("\n");

    for (i = 1U; i < n; i += 3U)
        bitarray_set_bits(&array, i, 2U);
    __print_bitarray(&array);

    n = sizeof(map) * 8U;
    for (i = 0U; i < n; i += 3U)
        bitarray_clear(&array, i);
    __print_bitarray(&array);

    printf("Bit Zero: ");
    for (i = 0U; i < n; ++i)
        printf(" %2ld", bitarray_find_zero(&array, i));
    printf("\n");

    printf("Bit Set:  ");
    for (i = 0U; i < n; ++i)
        printf(" %2ld", bitarray_find_set(&array, i));
    printf("\n");

    for (i = 1U; i < n; i += 3U)
        bitarray_clear_bits(&array, i, 2U);
    __print_bitarray(&array);


    return(0);
}

