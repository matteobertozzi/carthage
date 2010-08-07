/* [ mmblock.h ] - copy-on-write memory block
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

#ifndef _MMBLOCK_H_
#define _MMBLOCK_H_

#include <stdint.h>

typedef struct _mmallocator {
    void * (*alloc) (void *user_data, uint32_t n);
    void   (*free)  (void *user_data, void *ptr);
    void *user_data;
} mmallocator_t;

typedef struct _mmblock {
    void *alk;
    void *d;
} mmblock_t;

mmblock_t * mmblock_alloc    (mmblock_t *block,
                              mmallocator_t *allocator,
                              uint32_t n);
mmblock_t * mmblock_zalloc   (mmblock_t *block,
                              mmallocator_t *allocator,
                              uint32_t n);
void        mmblock_free     (mmblock_t *block);

void        mmblock_copy     (mmblock_t *dest,
                              const mmblock_t *src);

int32_t     mmblock_read     (mmblock_t *block,
                              void *buf,
                              uint32_t offset,
                              uint32_t n);
int32_t     mmblock_write    (mmblock_t *block,
                              const void *buf,
                              uint32_t offset,
                              uint32_t n);

#endif /* !_MMBLOCK_H_ */

