/* [ queue.h ] - Queue (FIFO) and Circular Queue
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

#include "queue.h"

#define __QUEUE_NITEMS                  (64)
#define __QUEUE_POOL_SIZE               (8)

#define __mmalk_data(queue)    ((queue)->alk->user_data)
#define __mmalloc(queue, n)    ((queue)->alk->alloc(__mmalk_data(queue), (n)))
#define __mmfree(queue, ptr)   ((queue)->alk->free(__mmalk_data(queue), (ptr)))

struct _queue_node {
    void *next;

    void *items[__QUEUE_NITEMS];
    uint8_t offset;
    uint8_t used;
};

/* Default stdlib allocator */
static void *__dmmalloc (void *x, uint32_t n) { return(malloc(n)); }
static void __dmmfree (void *x, void *ptr) { free(ptr); }
static mmallocator_t __default_mmallocator = {
    .alloc = __dmmalloc,
    .free  = __dmmfree,
    .user_data = NULL,
};

static queue_node_t *__queue_node_alloc (queue_t *queue) {
    queue_node_t *node;

    if (queue->pool != NULL) {
        node = queue->pool;
        queue->pool = node->next;
        queue->pool_size--;
    } else {
        node = (queue_node_t *) __mmalloc(queue, sizeof(queue_node_t));
        if (node == NULL)
            return(NULL);
    }

    node->next = NULL;
    node->offset = 0U;
    node->used = 0U;

    return(node);
}

static void __queue_node_free (queue_t *queue, queue_node_t *node) {
    if (queue->item_free != NULL) {
        while (node->used--)
            queue->item_free(queue->user_data, node->items[node->used]);
    }

    if (queue->pool_size < __QUEUE_POOL_SIZE) {
        node->next = queue->pool;
        queue->pool = node;
        queue->pool_size++;
    } else {
        __mmfree(queue, node);
    }
}

queue_t *queue_alloc (queue_t *queue,
                      mmallocator_t *allocator,
                      mmfree_t item_free,
                      void *user_data)
{
    return(queue_circular_alloc(queue, 0U, allocator, item_free, user_data));
}

queue_t *queue_circular_alloc (queue_t *queue,
                               size_t circular,
                               mmallocator_t *allocator,
                               mmfree_t item_free,
                               void *user_data)
{
    queue->item_free = item_free;
    queue->user_data = user_data;

    queue->alk = (allocator != NULL) ? allocator : &__default_mmallocator;

    queue->head = NULL;
    queue->tail = NULL;
    queue->pool = NULL;

    queue->pool_size = 0U;
    queue->circular = circular;
    queue->nitems = 0U;

    return(queue);
}

void queue_free (queue_t *queue) {
    queue_node_t *next;
    queue_node_t *node;

    /* Free Queue Nodes */
    for (node = queue->head; node != NULL; node = next) {
        next = node->next;
        __queue_node_free(queue, node);
    }

    /* Free Queue Pool */
    for (node = queue->pool; node != NULL; node = next) {
        next = node->next;
        __mmfree(queue, node);
    }

}

size_t queue_size (const queue_t *queue) {
    return(queue->nitems);
}

int queue_push (queue_t *queue,
                void *element)
{
    queue_node_t *node;
    void *x;

    if (queue->head == NULL) {
        /* Lazy allocation of queue first item */
        if ((queue->head = __queue_node_alloc(queue)) == NULL)
            return(-1);

        queue->tail = queue->head;
    } else if (queue->nitems == queue->circular && queue->nitems > 0) {
        /* Circular queue if full, pop one element */
        x = queue_pop(queue);
        if (queue->item_free != NULL)
            queue->item_free(queue->user_data, x);
    }

    node = queue->tail;
    if (node->used == __QUEUE_NITEMS) {
        if (node->offset > 0) {
            /* Lazy move back to offet 0 */
            x = node->items + node->offset;
            memcpy(x, x + 1, node->used * sizeof(void *));
            node->offset = 0;
        } else {
            /* Tail Node is full, we need another one */
            if ((node = __queue_node_alloc(queue)) == NULL)
                return(-2);

            queue->tail->next = node;
            queue->tail = node;
        }
    }

    /* Add Item to the queue */
    node->items[node->offset + node->used] = element;
    queue->nitems++;
    node->used++;

    return(0);
}

void *queue_pop (queue_t *queue) {
    queue_node_t *node;
    void *element;

    if (queue->nitems == 0)
        return(NULL);

    node = queue->head;
    element = node->items[node->offset++];
    queue->nitems--;
    node->used--;

    if (node->used == 0) {
        queue->head = node->next;
        __queue_node_free(queue, node);
    }

    return(element);
}

