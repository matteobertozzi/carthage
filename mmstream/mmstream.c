/* [ mmstream.c ] - Memory Stream
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
#include <stdint.h>
#include <stdlib.h>

#include "mmstream.h"

#if 0 /* Zero this if you don't wont bitmap check */
    #define USE_MMSTREAM_BITMAP
#endif

#define __FLOW_BLK_SHIFT          (9)
#define __FLOW_BLK_SIZE           (1 << __FLOW_BLK_SHIFT)

#if defined(USE_MMSTREAM_BITMAP)
    #define __FLOW_BITMAP_SIZE    (__FLOW_BLK_SIZE >> 3)

    #define __BITBLK(map, nr)     (*(((unsigned char *)map) + ((nr) >> 3)))
    #define __BITMASK(nr)         (1 << ((nr) & 0x7))

    #define __bit_set(map, nr)    (__BITBLK(map, nr) |= __BITMASK(nr))
    #define __bit_clear(map, nr)  (__BITBLK(map, nr) &= ~__BITMASK(nr))
    #define __bit_test(map, nr)   ((__BITBLK(map, nr) & __BITMASK(nr)) != 0)
#endif /* USE_MMSTREAM_BITMAP */

#define __FLOW_EXTENT_SHIFT       (6)
#define __FLOW_EXTENT_SIZE        (1 << __FLOW_EXTENT_SHIFT)

#define __align_down(x, align)    ((x) & (-(align)))
#define __align_up(x, align)      (((x) + ((align) - 1)) & (-(align)))
#define __min(a, b)               ((a) < (b) ? (a) : (b))

#define __mmalk_data(flow)        ((flow)->alk->user_data)
#define __mmalloc(flow, n)        ((flow)->alk->alloc(__mmalk_data(flow), (n)))
#define __mmfree(flow, ptr)       ((flow)->alk->free(__mmalk_data(flow), (ptr)))

typedef struct __mmlocation mmlocation_t;
typedef struct __mmblk mmblk_t;

struct __mmlocation {
    mmextent_t *extent;            /* Extent Pointer */
    uint64_t    ext_nr;            /* Extent Number */
    uint16_t    ext_off;           /* Extent Offset */
    uint16_t    blk_off;           /* Block Offset (Byte Offset) */
    uint16_t    blk_avail;         /* Available Size for this block */
};

struct __mmblk {
    uint8_t buf[__FLOW_BLK_SIZE];
#if defined(USE_MMSTREAM_BITMAP)
    uint8_t map[__FLOW_BITMAP_SIZE];
#endif /* USE_MMSTREAM_BITMAP */
};

struct __mmextent {
    mmextent_t *next;
    mmextent_t *prev;
    mmblk_t *   blocks[__FLOW_EXTENT_SIZE];
};

/* Default stdlib allocator */
static void *__dmmalloc (void *x, uint32_t n) { return(malloc(n)); }
static void __dmmfree (void *x, void *ptr) { free(ptr); }
static mmallocator_t __default_mmallocator = {
    .alloc = __dmmalloc,
    .free  = __dmmfree,
    .user_data = NULL,
};

/* ======================================================================
 *  Block
 */
static mmblk_t *__mmblk_alloc (mmstream_t *flow) {
    mmblk_t *block;

    if ((block = __mmalloc(flow, sizeof(mmblk_t))) == NULL)
        return(NULL);

    memset(block->buf, 0, __FLOW_BLK_SIZE);
    return(block);
}

static void __mmblk_free (mmstream_t *flow, mmblk_t *block) {
    __mmfree(flow, block);
}

#if defined(USE_MMSTREAM_BITMAP)
/* TODO: Optimize me for unsigned long block */
static void __mmblk_set_bits (mmblk_t *block,
                              uint32_t offset,
                              uint32_t size)
{
    while (size--)
        __bit_set(block->map, offset + size);
}

static void __mmblk_clear_bits (mmblk_t *block,
                                uint32_t offset,
                                uint32_t size)
{
    while (size--)
        __bit_clear(block->map, offset + size);
}

static uint32_t __mmblk_test_bits (mmblk_t *block,
                                   uint32_t offset,
                                   uint32_t size)
{
    uint32_t n = 0U;

    while (size-- && __bit_test(block->map, offset + n))
        n += 1;

    return(n);
}
#endif /* USE_MMSTREAM_BITMAP */

/* ======================================================================
 *  Extent
 */
static mmextent_t *__mmextent_alloc (mmstream_t *flow) {
    mmextent_t *extent;

    if ((extent = __mmalloc(flow, sizeof(mmextent_t))) != NULL) {
        memset(extent->blocks, 0, __FLOW_EXTENT_SIZE * sizeof(mmblk_t *));
        extent->next = NULL;
        extent->prev = NULL;
    }

    return(extent);
}

static void __mmextent_free (mmstream_t *flow, mmextent_t *extent) {
    uint32_t nblocks = __FLOW_EXTENT_SIZE;
    mmblk_t *block;

    while (nblocks--) {
        if ((block = extent->blocks[nblocks]) != NULL)
            __mmfree(flow, block);
    }

    __mmfree(flow, extent);
}

static int __mmextent_resize (mmstream_t *flow, uint64_t size) {
    mmextent_t *extent;
    uint64_t ext_nr;

    size = __align_up(size, __FLOW_BLK_SIZE);
    ext_nr = 1U + ((size >> __FLOW_BLK_SHIFT) >> __FLOW_EXTENT_SHIFT);
    if (flow->extent < ext_nr) {
        ext_nr = ext_nr - flow->extent;
        while (ext_nr--) {
            if ((extent = __mmextent_alloc(flow)) == NULL)
                return(-1);

            if (flow->tail != NULL)
                flow->tail->next = extent;
            else if (flow->head == NULL)
                flow->head = extent;

            extent->prev = flow->tail;
            flow->tail = extent;
            flow->extent++;
        }
    }

    return(0);
}

static mmextent_t *__mmextent_at (const mmstream_t *flow, uint64_t index) {
    uint64_t delta;
    mmextent_t *p;

    if (index == 0)
        return(flow->head);

    if (index == (flow->extent - 1))
        return(flow->tail);

    if ((delta = (flow->extent - index)) < (flow->extent >> 1)) {
        p = flow->tail;
        while (delta--)
            p = p->prev;
    } else {
        p = flow->head;
        while (index--)
            p = p->next;
    }

    return(p);
}

/* ======================================================================
 *  Flow Helpers
 */
static void __mmlocation_compute (mmstream_t *flow,
                                  mmlocation_t *location,
                                  uint64_t offset)
{
    uint64_t blk_nr;
    blk_nr = offset >> __FLOW_BLK_SHIFT;
    location->ext_nr  = blk_nr >> __FLOW_EXTENT_SHIFT;
    location->ext_off = blk_nr - __align_down(blk_nr, __FLOW_EXTENT_SIZE);
    location->blk_off = offset - __align_down(offset, __FLOW_BLK_SIZE);

    /* Compute Available Size, for this block */
    location->blk_avail = __align_up(offset, __FLOW_BLK_SIZE) - offset;
    if (location->blk_avail == 0) location->blk_avail = __FLOW_BLK_SIZE;

    location->extent = __mmextent_at(flow, location->ext_nr);
}

static int __mmlocation_next (const mmstream_t *flow,
                              mmlocation_t *location)
{
    if (++(location->ext_off) == __FLOW_EXTENT_SIZE) {
        location->ext_off = 0U;
        location->ext_nr++;
        location->extent = location->extent->next;
    }

    location->blk_off = 0U;
    location->blk_avail = __FLOW_BLK_SIZE;

    return(location->extent == NULL);
}

/* ======================================================================
 *  Flow
 */
mmstream_t *mmstream_alloc (mmstream_t *flow,
                            mmallocator_t *allocator)
{
    flow->alk = (allocator != NULL) ? allocator : &__default_mmallocator;
    flow->head = NULL;
    flow->tail = NULL;
    flow->extent = 0U;
    return(flow);
}

void mmstream_free (mmstream_t *flow) {
    mmstream_clear(flow);
}

void mmstream_clear (mmstream_t *flow) {
    mmextent_t *next;
    mmextent_t *p;

    for (p = flow->head; p != NULL; p = next) {
        next = p->next;
        __mmextent_free(flow, p);
    }

    flow->head = NULL;
    flow->tail = NULL;
    flow->extent = 0U;
}

void mmstream_truncate (mmstream_t *flow,
                        uint64_t size)
{
    mmlocation_t locat;
    mmextent_t *extent;
    mmextent_t *root;
    mmblk_t *block;

    __mmlocation_compute(flow, &locat, size);
    root = locat.extent;

    if (locat.blk_off > 0U || locat.ext_off > 0U) {
        do {
            if ((block = locat.extent->blocks[locat.ext_off]) != NULL) {
#if defined(USE_MMSTREAM_BITMAP)
                __mmblk_clear_bits(block, locat.blk_off, locat.blk_avail);
#endif /* USE_MMSTREAM_BITMAP */

                memset(block->buf + locat.blk_off, 0, locat.blk_avail);
            }

            __mmlocation_next(flow, &locat);
        } while (locat.extent == root);

        root = locat.extent;
    }

    while (locat.extent != NULL) {
        extent = locat.extent;
        locat.extent = extent->next;
        __mmextent_free(flow, extent);
        flow->extent--;
    }

    if (root != NULL) {
        if (root->prev != NULL)
            root->prev->next = NULL;
        flow->tail = root;
    }
}

int32_t mmstream_read (mmstream_t *flow,
                       void *buffer,
                       uint64_t offset,
                       uint32_t size)
{
    uint8_t *p = (uint8_t *)buffer;
    mmlocation_t locat;
    uint32_t fetchz;
    mmblk_t *block;
    int32_t n = 0;

    /* Trim return overflow part */
    size &= 0x7fffffffU;

    __mmlocation_compute(flow, &locat, offset);
    while (n < size) {
        if ((block = locat.extent->blocks[locat.ext_off]) == NULL)
            break;

        fetchz = __min(size - n, locat.blk_avail);
#if defined(USE_MMSTREAM_BITMAP)
        fetchz = __mmblk_test_bits(block, locat.blk_off, fetchz);
#endif /* USE_MMSTREAM_BITMAP */

        if (fetchz == 0)
            break;

        memcpy(p, block->buf + locat.blk_off, fetchz);
        p += fetchz;
        n += fetchz;

#if defined(USE_MMSTREAM_BITMAP)
        if (fetchz < __min(size - n, locat.blk_avail))
            break;
#endif /* USE_MMSTREAM_BITMAP */

        if (__mmlocation_next(flow, &locat))
            break;
    }

    return(n);
}

int32_t mmstream_write (mmstream_t *flow,
                        const void *buffer,
                        uint64_t offset,
                        uint32_t size)
{
    mmlocation_t locat;
    uint32_t fetchz;
    mmblk_t *block;
    int32_t n = 0;
    uint8_t *p;

    /* Trim return overflow part */
    size &= 0x7fffffffU;
    if (size == 0) return(0);

    /* If we've not enough extent, allocate new ones */
    if (__mmextent_resize(flow, offset))
        return(-1);

    __mmlocation_compute(flow, &locat, offset);
    while (n < size) {
        if ((fetchz = __min(size - n, locat.blk_avail)) == 0)
            break;

        if ((block = locat.extent->blocks[locat.ext_off]) == NULL) {
            if ((block = __mmblk_alloc(flow)) == NULL)
                break;

            locat.extent->blocks[locat.ext_off] = block;
        }

#if defined(USE_MMSTREAM_BITMAP)
        __mmblk_set_bits(block, locat.blk_off, fetchz);
#endif /* USE_MMSTREAM_BITMAP */

        p = block->buf + locat.blk_off;
        memcpy(p, buffer, fetchz);
        buffer += fetchz;
        n += fetchz;

        if (__mmextent_resize(flow, offset + n))
            break;

        if (__mmlocation_next(flow, &locat))
            break;
    }

    return(n);
}

