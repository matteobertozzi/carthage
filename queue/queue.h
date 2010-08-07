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

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdint.h>
#include <stddef.h>

typedef void * (*mmalloc_t) (void *user_data, uint32_t n);
typedef void   (*mmfree_t)  (void *user_data, void *ptr);

typedef struct _mmallocator {
    mmalloc_t alloc;
    mmfree_t  free;
    void *    user_data;
} mmallocator_t;

typedef struct _queue_node queue_node_t;

typedef struct _queue {
    mmfree_t       item_free;
    void         * user_data;

    mmallocator_t *alk;

    queue_node_t * head;
    queue_node_t * tail;

    queue_node_t * pool;
    size_t         pool_size;

    size_t         circular;
    size_t         nitems;
} queue_t;


queue_t *   queue_alloc               (queue_t *queue,
                                       mmallocator_t *allocator,
                                       mmfree_t item_free,
                                       void *user_data);
queue_t *   queue_circular_alloc      (queue_t *queue,
                                       size_t circular,
                                       mmallocator_t *allocator,
                                       mmfree_t item_free,
                                       void *user_data);
void        queue_free                (queue_t *queue);

size_t      queue_size                (const queue_t *queue);

int         queue_push                (queue_t *queue,
                                       void *element);
void *      queue_pop                 (queue_t *queue);

#endif /* !_QUEUE_H_ */

