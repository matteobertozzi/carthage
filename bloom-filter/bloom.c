/* [ bloom.c ] - Bloom filter
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
#include <stdlib.h>

#include "bloom.h"

#define __BITMASK(n)             (1 << ((n) & 0x7))
#define __BIT(map, n)            (*(((unsigned char *)(map)) + ((n) >> 3)))

#define __bit_set(map, n)        do { __BIT(map, n) |= __BITMASK(n); } while (0)
#define __bit_test(map, n)       (!!(__BIT(map, n) & __BITMASK(n)))

#define __mmalloc(bloom, n)      ((uint8_t *)malloc(n))
#define __mmfree(bloom, ptr)     free(ptr)

#define __bloom_bits(bloom)      ((bloom)->num_slices * (bloom)->bits_per_slice)
#define __bloom_bytes(bloom)     (__bloom_bits(bloom) >> 3)

#define __bloom_hash(bloom, k, h)      \
    ((bloom)->hash_func(k, h, (bloom)->num_slices, (bloom)->bits_per_slice))


int bloom_alloc (bloom_t *bloom,
                 unsigned int capacity,
                 unsigned int num_slices,
                 unsigned int bits_per_slice,
                 bloom_hash_t hash_func)
{
    unsigned int nbytes;

    if (num_slices > BLOOM_MAX_SLICES)
        return(-1);

    bloom->bits_per_slice = bits_per_slice;
    bloom->num_slices = num_slices;
    bloom->hash_func = hash_func;
    bloom->capacity = capacity;
    bloom->count = 0U;

    nbytes = __bloom_bytes(bloom);
    if ((bloom->bitarray = __mmalloc(bloom, nbytes)) == NULL)
        return(-2);

    memset(bloom->bitarray, 0, nbytes);

    return(0);
}

void bloom_free (bloom_t *bloom) {
    __mmfree(bloom, bloom->bitarray);
}

int bloom_add (bloom_t *bloom, const void *key) {
    uint32_t hashes[BLOOM_MAX_SLICES];
    unsigned int offset;
    unsigned int hi;

    if (bloom->count >= bloom->capacity)
        return(1);

    if (__bloom_hash(bloom, key, hashes))
        return(-1);

    offset = 0U;
    hi = bloom->num_slices;
    while (hi--) {
        __bit_set(bloom->bitarray, offset + hashes[hi]);
        offset += bloom->bits_per_slice;
    }

    bloom->count++;
    return(0);
}

/**
 * Test whether an element is a member of a set.
 * Returns < 0 on error
 * Returns 0 if key is not in.
 * Returns 1 if key is in.
 * False positives are possible, but false negatives are not.
 */
int bloom_contains (const bloom_t *bloom, const void *key) {
    uint32_t hashes[BLOOM_MAX_SLICES];
    unsigned int offset;
    unsigned int hi;

    if (__bloom_hash(bloom, key, hashes))
        return(-1);

    offset = 0U;
    hi = bloom->num_slices;
    while (hi--) {
        if (!__bit_test(bloom->bitarray, offset + hashes[hi]))
            return(0);

        offset += bloom->bits_per_slice;
    }

    return(1);
}

