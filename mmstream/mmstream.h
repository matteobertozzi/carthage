/* [ mmstream.h ] - Memory Stream
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

#ifndef _MMSTREAM_H_
#define _MMSTREAM_H_

#include <stdint.h>

typedef struct __mmextent mmextent_t;

typedef void * (*mmalloc_t) (void *user_data, uint32_t n);
typedef void   (*mmfree_t)  (void *user_data, void *ptr);

typedef struct _mmallocator {
    mmalloc_t alloc;
    mmfree_t  free;
    void *    user_data;
} mmallocator_t;

typedef struct __mmstream {
    mmallocator_t *alk;
    mmextent_t *head;
    mmextent_t *tail;
    uint64_t    extent;
    /* TODO: Add Pool for blk and extent */
} mmstream_t;

mmstream_t *mmstream_alloc      (mmstream_t *flow,
                                 mmallocator_t *allocator);
void        mmstream_free       (mmstream_t *flow);

void        mmstream_clear      (mmstream_t *flow);
void        mmstream_truncate   (mmstream_t *flow,
                                 uint64_t size);

int32_t     mmstream_read       (mmstream_t *flow,
                                 void *buffer,
                                 uint64_t offset,
                                 uint32_t size);
int32_t     mmstream_write      (mmstream_t *flow,
                                 const void *buffer,
                                 uint64_t offset,
                                 uint32_t size);

#endif /* !_MMSTREAM_H_ */

