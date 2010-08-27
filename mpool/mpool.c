/*
 * [ mpool.h ] - Memory Pool
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
 *
 * mpool is a simple Memory Pool with 32bit overhead per block.
 * Scan blocks and find the first free-block with the same size.
 *
 * +------+----------------------+------+-------------+------+----------------+
 * | size | block                | size | block       | size | block          |
 * +------+----------------------+------+-------------+------+----------------+
 */

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "mpool.h"

typedef struct _mchunk {
    uint32_t size;
} mchunk_t;

/**
 * Initialize mem-pool with specified size memory-block.
 */
void mpool_init (mpool_t *mpool, void *mem, uint32_t size) {
    mchunk_t *mchunk;

    mpool->mem = mem;
    mpool->size = size;

    /* Init free size and first chunk, marked as free */
    mpool->free = size - sizeof(mchunk_t);
    mchunk = (mchunk_t *)mem;
    mchunk->size = mpool->free & 0x7fffffff;
}

/**
 * Alloc new block with specified size.
 */
void *mpool_alloc (mpool_t *mpool, uint32_t size) {
    uint32_t chunk_size;
    mchunk_t *nchunk;
    mchunk_t *mchunk;
    uint8_t *p, *end;

    p = mpool->mem;
    end = p + mpool->size;
    while (p < end) {
        mchunk = (mchunk_t *)p;
        chunk_size = mchunk->size & 0x7fffffff;

        /* If this block is used skip next */
        if (mchunk->size >> 31)
            goto _next_mchunk;

        /* Try to merge free blocks if size is not enough*/
        while (chunk_size < size) {
            nchunk = (mchunk_t *)(p + chunk_size + sizeof(mchunk_t));
            if ((uint8_t *)nchunk < end && nchunk->size >> 31) {
                p = (uint8_t *)nchunk;
                chunk_size = nchunk->size & 0x7fffffff;
                goto _next_mchunk;
            } else if ((uint8_t *)nchunk >= end) {
                return(NULL);
            }

            /* Merge two chunks */
            mpool->free += sizeof(mchunk_t);
            mchunk->size += nchunk->size + sizeof(mchunk_t);
            chunk_size = mchunk->size & 0x7fffffff;
        }

        /* At this point we've chunk that can handle our request */
        assert(chunk_size >= size);

        /* Allocate chunk */
        mchunk->size = size | 0x80000000;

        /* Setup Next Chunk if this one has more than requested space */
        if (chunk_size > size) {
            mchunk = (mchunk_t *)(p + sizeof(mchunk_t) + size);
            mchunk->size = (chunk_size - size - sizeof(mchunk_t)) & 0x7fffffff;
            mpool->free -= sizeof(mchunk_t);
        }

        mpool->free -= size;
        return(p + sizeof(mchunk_t));

_next_mchunk:
        p += chunk_size + sizeof(mchunk_t);
    }

    return(NULL);
}

/**
 * Release block allocated by this mpool.
 */
void mpool_free (mpool_t *mpool, void *block) {
    mchunk_t *mchunk;
    mchunk_t *ochunk;

    if (block == NULL)
        return;

    /* Mark the block as free, and update pool free size */
    mchunk = (mchunk_t *)(block - sizeof(mchunk_t));
    mchunk->size &= 0x7fffffff;
    mpool->free += mchunk->size;

    /* Merge Next Chunk if it's free */
    ochunk = (mchunk_t *)(block + mchunk->size);
    if (!(ochunk->size >> 31)) {
        mchunk->size += ochunk->size + sizeof(mchunk_t);
        mpool->free += sizeof(mchunk_t);
    }
}

/**
 * Re-Alloc new block with specified size.
 * if block is NULL: mpool_alloc() is called.
 * if size is 0: mpool_free() is called and NULL returned.
 * If block resize fail, NULL is returned but block is still alive.
 */
void *mpool_realloc (mpool_t *mpool, void *block, uint32_t size) {
    uint32_t ochunk_size;
    uint32_t chunk_size;
    uint32_t diff_size;
    mchunk_t *mchunk;
    mchunk_t *ochunk;

    /* Same as mpool_alloc() */
    if (block == NULL)
        return(mpool_alloc(mpool, size));

    /* Same as mpool_free() */
    if (size == 0U) {
        mpool_free(mpool, block);
        return(NULL);
    }

    mchunk = (mchunk_t *)(block - sizeof(mchunk_t));
    chunk_size = mchunk->size & 0x7fffffff;

    if (size < chunk_size) {
        diff_size = (chunk_size - size);

        /* You want drop too few space... keep this block */
        if (diff_size <= (sizeof(mchunk_t) << 1))
            return(block);

        /* Setup Next Chunk and Set New Size */
        ochunk = (mchunk_t *)(block + size);
        ochunk->size = (mchunk->size - size - sizeof(mchunk_t)) & 0x7fffffff;
        mchunk->size = size | 0x80000000;
        mpool->free += diff_size - sizeof(mchunk_t);
    } else {
        diff_size = (size - chunk_size);

        ochunk = (mchunk_t *)(block + chunk_size);
        ochunk_size = ochunk->size & 0x7fffffff;
        if ((ochunk->size >> 31) ||
            (diff_size >= (ochunk->size + sizeof(mchunk_t))))
        {
            void *nblock;

            /* Next block is not free, try with malloc() */
            if ((nblock = mpool_alloc(mpool, size)) != NULL) {
                memcpy(nblock, block, mchunk->size);
                mpool_free(mpool, block);
            }

            return(nblock);
        }

        /* Merge this and next block */
        mchunk->size += ochunk_size + sizeof(mchunk_t);
        ochunk_size = (mchunk->size & 0x7fffffff) - size;
        mpool->free -= diff_size;

        if (ochunk_size <= (sizeof(mchunk_t) << 1)) {
            mpool->free -= sizeof(mchunk_t);
            return(block);
        }

        /* Setup Next Chunk and Set New Size */
        ochunk = (mchunk_t *)(block + size);
        ochunk->size = (ochunk_size - sizeof(mchunk_t)) & 0x7fffffff;
        mchunk->size = size | 0x80000000;
    }

    return(block);
}

