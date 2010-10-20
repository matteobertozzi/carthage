/* [ bloom.h ] - Bloom filter
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

#ifndef _BLOOM_FILTER_H_
#define _BLOOM_FILTER_H_

#include <stdint.h>

#define BLOOM_MAX_SLICES        (8)

typedef int (*bloom_hash_t) (const void *key,
                             uint32_t *hashes,
                             unsigned int nhashes,
                             unsigned int max_value);

typedef struct _bloom {
    bloom_hash_t hash_func;
    unsigned int bits_per_slice;
    unsigned int num_slices;
    unsigned int capacity;
    unsigned int count;
    uint8_t *bitarray;
} bloom_t;

int     bloom_alloc     (bloom_t *bloom,
                         unsigned int capacity,
                         unsigned int num_slices,
                         unsigned int bits_per_slice,
                         bloom_hash_t hash_func);

void    bloom_free      (bloom_t *bloom);

int     bloom_add       (bloom_t *bloom,
                         const void *key);

int     bloom_contains  (const bloom_t *bloom,
                         const void *key);

#endif /* _BLOOM_FILTER_H_ */

