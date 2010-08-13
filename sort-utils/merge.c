/* [ merge.c ] - Merge
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
#include "merge.h"

void merge (void *r, size_t rs,
            const void *a, size_t as,
            const void *b, size_t bs,
            size_t obj_size,
            itemcmp_t cmp_func,
            itemcpy_t cpy_func,
            void *user_data)
{
    const void *ae = a + (as * obj_size);
    const void *be = b + (bs * obj_size);
    const void *re = r + (rs * obj_size);

    while (a < ae && b < be && r < re) {
        if (cmp_func(user_data, a, b, obj_size) < 0) {
            cpy_func(user_data, r, a, obj_size);
            a += obj_size;
        } else {
            cpy_func(user_data, r, b, obj_size);
            b += obj_size;
        }
        r += obj_size;
    }

    while (a < ae && r < re) {
        cpy_func(user_data, r, a, obj_size);
        r += obj_size;
        a += obj_size;
    }

    while (b < be && r < re) {
        cpy_func(user_data, r, b, obj_size);
        r += obj_size;
        b += obj_size;
    }
}

void mmmerge (void *r, size_t rs,
              const void *a, size_t as,
              const void *b, size_t bs,
              size_t obj_size)
{
    const void *ae = a + (as * obj_size);
    const void *be = b + (bs * obj_size);
    const void *re = r + (rs * obj_size);

    while (a < ae && b < be && r < re) {
        if (memcmp(a, b, obj_size) < 0) {
            memcpy(r, a, obj_size);
            a += obj_size;
        } else {
            memcpy(r, b, obj_size);
            b += obj_size;
        }
        r += obj_size;
    }

    if (a < ae && r < re) {
        obj_size = (ae - a) < (re - r) ? (ae - a) : (re - r);
        memcpy(r, a, obj_size);
        r += obj_size;
    }

    if (b < be && r < re) {
        obj_size = (be - b) < (re - r) ? (be - b) : (re - r);
        memcpy(r, b, obj_size);
    }
}

