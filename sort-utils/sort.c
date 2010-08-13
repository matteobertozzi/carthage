/* [ sort.c ] - Sort
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
#include "sort.h"

static void __memswap (void *a, void *b, size_t n) {
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

void sort (void *base,
           size_t nitems,
           size_t obj_size,
           itemcmp_t cmp_func,
           itemswp_t swp_func,
           void *user_data)
{
    unsigned char *p = (unsigned char *)base;
    long i = ((nitems >> 1) - 1) * obj_size;
    size_t n = (nitems * obj_size);
    size_t c, r;

    /* heapify */
    for (; i >= 0; i -= obj_size) {
        for (r = i; ((r << 1) + obj_size) < n; r = c) {
            c = (r << 1) + obj_size;
            if (c < (n - obj_size) &&
                cmp_func(user_data, p + c, p + c + obj_size, obj_size) < 0)
            {
                c += obj_size;
            }

            if (cmp_func(user_data, p + r, p + c, obj_size) >= 0)
                break;

            swp_func(user_data, p + r, p + c, obj_size);
        }
    }

    /* sort */
    for (i = n - obj_size; i > 0; i -= obj_size) {
        swp_func(user_data, p, p + i, obj_size);
        for (r = 0; ((r << 1) + obj_size) < i; r = c) {
            c = (r << 1) + obj_size;
            if (c < (i - obj_size) &&
                cmp_func(user_data, p + c, p + c + obj_size, obj_size) < 0)
            {
                c += obj_size;
            }

            if (cmp_func(user_data, p + r, p + c, obj_size) >= 0)
                break;

            swp_func(user_data, p + r, p + c, obj_size);
        }
    }
}


void mmsort (void *base, size_t nitems, size_t obj_size) {
    unsigned char *p = (unsigned char *)base;
    long i = ((nitems >> 1) - 1) * obj_size;
    size_t n = (nitems * obj_size);
    size_t c, r;

    /* heapify */
    for (; i >= 0; i -= obj_size) {
        for (r = i; ((r << 1) + obj_size) < n; r = c) {
            c = (r << 1) + obj_size;
            if (c < (n - obj_size) && memcmp(p+c, p+c+obj_size, obj_size) < 0)
                c += obj_size;

            if (memcmp(p + r, p + c, obj_size) >= 0)
                break;

            __memswap(p + r, p + c, obj_size);
        }
    }

    /* sort */
    for (i = n - obj_size; i > 0; i -= obj_size) {
        __memswap(p, p + i, obj_size);
        for (r = 0; ((r << 1) + obj_size) < i; r = c) {
            c = (r << 1) + obj_size;
            if (c < (i - obj_size) && memcmp(p+c, p+c+obj_size, obj_size) < 0)
                c += obj_size;
            if (memcmp(p + r, p + c, obj_size) >= 0)
                break;
            __memswap(p + r, p + c, obj_size);
        }
    }
}

