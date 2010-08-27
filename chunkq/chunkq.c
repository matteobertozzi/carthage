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

chunkq_t *chunkq_alloc (chunkq_t *chunkq,  size_t chunk_size) {
    chunkq->head = NULL;
    chunkq->tail = NULL;
    chunkq->pool = NULL;
    chunkq->psize = 0U;
    chunkq->chunk = chunk_size;
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
    uint8_t *p = (uint8_t *)buffer;
    chunkn_t *node;
    size_t wr = 0;
    size_t bksize;

    if ((node = chunkq->tail) == NULL) {
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
        wr += bksize;
        p += bksize;
    }

    return(wr);
}

