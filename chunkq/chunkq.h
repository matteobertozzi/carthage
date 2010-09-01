/* [ chunkq.h ] - Chunk Queue
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

#ifndef _CHUNKQ_H_
#define _CHUNKQ_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct _chunkq {
    void *   head;
    void *   tail;
    void *   pool;
    uint32_t psize;
    uint32_t chunk;
    size_t   size;
} chunkq_t;

chunkq_t *chunkq_alloc      (chunkq_t *chunkq,
                             uint32_t chunk_size);
void      chunkq_free       (chunkq_t *chunkq);

void      chunkq_clear      (chunkq_t *chunkq);

ssize_t   chunkq_read       (chunkq_t *chunkq,
                             void *buffer,
                             size_t size);

ssize_t   chunkq_append     (chunkq_t *chunkq,
                             const void *buffer,
                             size_t size);

ssize_t   chunkq_peek       (chunkq_t *chunkq,
                             size_t offset,
                             void *buffer,
                             size_t size);
ssize_t    chunkq_indexof   (chunkq_t *chunkq,
                             size_t offset,
                             const void *needle,
                             size_t needle_len);

#endif /* _CHUNKQ_H_ */

