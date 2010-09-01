/* [ chunkq.c ] - Chunk Queue
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "chunkq.h"

#define __CHUNK_POOL            (8)
#define __CHUNKN(x)             ((chunkn_t *)(x))

#define __mmalloc(chunkq, n)    malloc((n))
#define __mmfree(chunkq, ptr)   free(ptr)

typedef struct _chunkn chunkn_t;

struct _chunkn {
    chunkn_t *next;
    uint8_t * data;
    uint32_t  offset;
    uint32_t  size;
};


struct chunkq_search {
    ssize_t offset;
    ssize_t rd;
    int state;

    const uint8_t *needle;
    uint32_t needle_len;

    const uint8_t *data;
    uint32_t data_len;
};

static chunkn_t *__chunkn_alloc (chunkq_t *chunkq) {
    chunkn_t *node;
    uint8_t *blob;

    if (chunkq->pool != NULL) {
        node = __CHUNKN(chunkq->pool);
        chunkq->pool = node->next;
        chunkq->psize--;
    } else {
        if ((blob = __mmalloc(chunkq, sizeof(chunkn_t)+chunkq->chunk)) == NULL)
            return(NULL);

        node = (chunkn_t *)blob;
        node->data = blob + sizeof(chunkn_t);
    }

    node->offset = 0U;
    node->size = 0U;
    node->next = NULL;

    return(node);
}

static void __chunkn_free (chunkq_t *chunkq, chunkn_t *node) {
    if (chunkq->psize > __CHUNK_POOL) {
        __mmfree(chunkq, node);
    } else {
        node->next = __CHUNKN(chunkq->pool);
        chunkq->pool = node;
    }
}

static void __chunkq_free (chunkq_t *chunkq, chunkn_t *head) {
    chunkn_t *next;

    for (; head != NULL; head = next) {
        next = head->next;
        __chunkn_free(chunkq, head);
    }
}

static chunkn_t *__chunkn_at_offset (chunkq_t *chunkq,
                                     size_t offset,
                                     size_t *x)
{
    chunkn_t *node;
    size_t n = 0U;

    node = __CHUNKN(chunkq->head);
    while (node != NULL) {
        n += node->size;
        if (n > offset)
            break;

        node = node->next;
    }

    *x = n;
    return(node);
}

static int __chunkn_search_step (struct chunkq_search *search,
                                 const uint8_t *needle,
                                 uint32_t needle_len)
{
    ssize_t index = -1;
    void *p;

    while ((p = memchr(search->data, *(search->needle), search->data_len))) {
        if ((index = ((uint8_t *)p - search->data) + 1) > 1 && search->state) {
            search->needle_len = needle_len;
            search->needle = (const uint8_t *)needle;
            search->state = 0;
            continue;
        }

        search->rd += index;
        if (!search->state) {
            search->offset = search->rd - 1;
            search->state = 1;
        }

        if (--(search->needle_len) == 0)
            return(1);

        search->needle++;
        search->data_len -= index;
        search->data += index;

        while (search->data_len > 0) {
            search->rd++;

            if (*(search->data) != *(search->needle)) {
                search->needle_len = needle_len;
                search->needle = (const uint8_t *)needle;
                search->state = 0;
                break;
            }

            if (--(search->needle_len) == 0)
                return(2);

            search->needle++;
            search->data++;
            search->data_len--;
        }

        if (search->data_len == 0)
            return(0);
    }

    if (index < 0)
        search->rd += search->data_len;

    return(0);
}

chunkq_t *chunkq_alloc (chunkq_t *chunkq,  uint32_t chunk_size) {
    chunkq->head = NULL;
    chunkq->tail = NULL;
    chunkq->pool = NULL;
    chunkq->psize = 0U;
    chunkq->chunk = chunk_size;
    chunkq->size = 0U;
    return(chunkq);
}

void chunkq_free (chunkq_t *chunkq) {
    chunkq_clear(chunkq);

    __chunkq_free(chunkq, __CHUNKN(chunkq->pool));
    chunkq->pool = NULL;
    chunkq->psize = 0U;
}

void chunkq_clear (chunkq_t *chunkq) {
    __chunkq_free(chunkq, __CHUNKN(chunkq->head));
    chunkq->head = NULL;
    chunkq->tail = NULL;
    chunkq->size = 0U;
}

ssize_t chunkq_read (chunkq_t *chunkq, void *buffer, size_t size) {
    uint8_t *p = (uint8_t *)buffer;
    chunkn_t *node;
    size_t rd = 0;
    size_t bksize;

    if ((node = __CHUNKN(chunkq->head)) == NULL)
        return(-1);

    while (rd < size) {
        bksize = (size - rd);
        if (node->size < bksize)
            bksize = node->size;

        memcpy(p, node->data + node->offset, bksize);

        p += bksize;
        rd += bksize;
        chunkq->size -= bksize;
        node->offset += bksize;
        node->size -= bksize;
        if (node->size == 0) {
            if ((chunkq->head = node->next) == NULL)
                chunkq->tail = NULL;
            __chunkn_free(chunkq, node);

            if ((node = chunkq->head) == NULL)
                break;
        }
    }

    return(rd);
}

ssize_t chunkq_append (chunkq_t *chunkq,
                       const void *buffer,
                       size_t size)
{
    const uint8_t *p = (const uint8_t *)buffer;
    chunkn_t *node;
    size_t wr = 0;
    size_t bksize;

    if ((node = __CHUNKN(chunkq->tail)) == NULL) {
        if ((node = __chunkn_alloc(chunkq)) == NULL)
            return(-1);
        chunkq->tail = node;
        chunkq->head = node;
    }

    while (wr < size) {
        if ((bksize = (chunkq->chunk - (node->offset + node->size))) == 0) {
            if ((node = __chunkn_alloc(chunkq)) == NULL)
                return(wr);

            __CHUNKN(chunkq->tail)->next = node;
            chunkq->tail = node;
            bksize = chunkq->chunk;
        }

        if ((size - wr) < bksize)
            bksize = (size - wr);

        memcpy(node->data + node->offset + node->size, p, bksize);
        node->size += bksize;
        chunkq->size += bksize;
        wr += bksize;
        p += bksize;
    }

    return(wr);
}

ssize_t chunkq_peek (chunkq_t *chunkq,
                     size_t offset,
                     void *buffer,
                     size_t size)
{
    uint8_t *p = (uint8_t *)buffer;
    chunkn_t *node;
    size_t bksize;
    size_t x;

    if ((node = __chunkn_at_offset(chunkq, offset, &x)) == NULL)
        return(-1);

    offset = offset - (x - node->size);
    if ((bksize = (node->size - offset)) > size)
        bksize = size;
    memcpy(p, node->data + node->offset + offset, bksize);
    p += bksize;
    x = bksize;

    while (x < size) {
        if ((node = node->next) == NULL)
            break;

        bksize = (size - x);
        if (node->size < bksize)
            bksize = node->size;

        memcpy(p, node->data, bksize);
        p += bksize;
        x += bksize;
    }

    return(x);
}

ssize_t chunkq_indexof (chunkq_t *chunkq,
                        size_t offset,
                        const void *needle,
                        size_t needle_len)
{
    struct chunkq_search search;
    chunkn_t *node;
    size_t x;

    if ((node = __chunkn_at_offset(chunkq, offset, &x)) == NULL)
        return(-1);

    search.state = 0;
    search.offset = 0;
    search.rd = offset;
    search.needle = needle;
    search.needle_len = needle_len;

    offset = offset - (x - node->size);
    search.data = (node->data + node->offset + offset);
    search.data_len = (node->size - offset);
    if (__chunkn_search_step(&search, needle, needle_len))
        return(search.offset);

    while ((node = node->next) != NULL) {
        search.data = node->data;
        search.data_len = node->size;
        if (__chunkn_search_step(&search, needle, needle_len))
            return(search.offset);
    }

    return(-1);
}

