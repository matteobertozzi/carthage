/* [ mmblock.c ] - copy-on-write memory block
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
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include "mmblock.h"

#define __MMIBLOCK(x)              ((mmiblock_t *)((x)->d))

#define __MMALK(blk)               ((mmallocator_t *)((blk)->alk))
#define __MMALK_DATA(blk)          (__MMALK(blk)->user_data)

#define __mmblock_alloc(blk, n)    (__MMALK(blk)->alloc(__MMALK_DATA(blk), n))
#define __mmblock_free(blk, x)     (__MMALK(blk)->free(__MMALK_DATA(blk), x))

typedef struct _mmiblock {
    uint8_t *data;
    uint32_t size;
    int      ref;
} mmiblock_t;

/* Default stdlib allocator */
static void *__mmalloc (void *x, uint32_t n) { return(malloc(n)); }
static void __mmfree (void *x, void *ptr) { free(ptr); }
static mmallocator_t __default_mmallocator = {
    .alloc = __mmalloc,
    .free  = __mmfree,
    .user_data = NULL,
};

static mmiblock_t *__mmiblock_alloc (mmblock_t *block, uint32_t n) {
    mmiblock_t *d;

    if ((d = __mmblock_alloc(block, sizeof(mmiblock_t) + n)) == NULL)
        return(NULL);

    /* Initialize New Block */
    d->data = ((uint8_t *)d) + sizeof(mmiblock_t);
    d->size = n;
    d->ref = 1;

    return(d);
}

static mmiblock_t *__mmiblock_detach (mmblock_t *block,
                                      uint32_t offset,
                                      uint32_t n)
{
    mmiblock_t *d = __MMIBLOCK(block);
    mmiblock_t *x;
    uint32_t ne;

    if ((x = __mmiblock_alloc(block, d->size)) == NULL)
        return(NULL);

    /* We've to detach bytes from offset to offset+n
     * But for this release we copy all the block... This is a FIXME.
     *          copy from 0 to (offset - 1)
     *          copy from (offset + n + 1) to (data_size)
     */
    ne = offset + n;
    memcpy(x->data, d->data, offset);
    memcpy(x->data + ne, d->data + ne, x->size - ne);

    block->d = x;
    if ((--(d->ref)) == 0)
        __mmblock_free(block, d);

    return(x);
}

/*
 * Allocate a new block with specified size (n).
 * If allocator is NULL, default malloc() and free() allocator is used.
 * else your allocator is called:
 *      allocator->alloc(allocator->user_data, size)
 *      allocator->free(allocator->user_data, ptr)
 */
mmblock_t *mmblock_alloc (mmblock_t *block,
                          mmallocator_t *allocator,
                          uint32_t n)
{
    block->alk = (allocator == NULL) ? &__default_mmallocator : allocator;
    if ((block->d = __mmiblock_alloc(block, n)) == NULL)
        return(NULL);

    return(block);
}

mmblock_t *mmblock_zalloc (mmblock_t *block,
                           mmallocator_t *allocator,
                           uint32_t n)
{
    if (mmblock_alloc(block, allocator, n) != NULL)
        memset(__MMIBLOCK(block)->data, 0, __MMIBLOCK(block)->size);
    return(block);
}

void mmblock_free (mmblock_t *block) {
    mmiblock_t *d = __MMIBLOCK(block);

    if ((--(d->ref)) == 0)
        __mmblock_free(block, d);
}

/*
 * Copy block src to block dest.
 * This is not a real copy. mmblock is copy-on-write.
 */
void mmblock_copy (mmblock_t *dest,
                   const mmblock_t *src)
{
    ++(__MMIBLOCK(src)->ref);
    dest->alk = src->alk;
    dest->d = src->d;
}

int32_t mmblock_read (mmblock_t *block,
                      void *buf,
                      uint32_t offset,
                      uint32_t n)
{
    mmiblock_t *d = __MMIBLOCK(block);
    int32_t rn;

    if (offset > d->size)
        return(-EFAULT);

    rn = d->size - offset;
    if ((rn = (rn < n) ? rn : n) == 0)
        return(rn);

    memcpy(buf, d->data + offset, rn);

    return(rn);
}

int32_t mmblock_write (mmblock_t *block,
                       const void *buf,
                       uint32_t offset,
                       uint32_t n)
{
    mmiblock_t *d = __MMIBLOCK(block);
    int32_t wn;

    if (offset > d->size)
        return(-EFAULT);

    wn = d->size - offset;
    if ((wn = (wn < n) ? wn : n) == 0)
        return(wn);

    if (d->ref != 1) {
        if ((d = __mmiblock_detach(block, offset, n)) == NULL)
            return(-ENOMEM);
    }

    memcpy(d->data + offset, buf, wn);

    return(wn);
}

