/* [ bitarray.h ] - BitArray (Bitmap) Utils
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

#ifndef _BITARRAY_H_
#define _BITARRAY_H_

#include <sys/types.h>

#define __BITMASK(n)          (1 << ((n) & 0x7))
#define __BIT(map, n)         (((unsigned char *)(map)) + ((n) >> 3))

#define bit_set(map, n)       do { *__BIT(map, n) |= __BITMASK(n); } while (0)
#define bit_clear(map, n)     do { *__BIT(map, n) &= ~__BITMASK(n); } while (0)
#define bit_test(map, n)      (*__BIT(map, n) & __BITMASK(n))

typedef struct _bitarray {
    unsigned char *map;
    size_t size;
} bitarray_t;

int     bitarray_set        (bitarray_t *array, size_t n);
int     bitarray_clear      (bitarray_t *array, size_t n);
int     bitarray_test       (bitarray_t *array, size_t n);

void    bitarray_set_bits   (bitarray_t *array, size_t start, size_t count);
void    bitarray_clear_bits (bitarray_t *array, size_t start, size_t count);

ssize_t bitarray_find_set   (bitarray_t *array, size_t offset);
ssize_t bitarray_find_zero  (bitarray_t *array, size_t offset);

#endif /* _BITARRAY_H_ */

