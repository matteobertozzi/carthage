/* [ bag.h ] - Bag
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

#ifndef _BAG_H_
#define _BAG_H_

#include <stddef.h>
#include <stdint.h>

typedef struct _bag_node bagnode_t;

typedef void   (*bag_foreach_t) (void *user_data,
                                 const void *key,
                                 size_t count);
typedef size_t (*bag_hash_t)    (void *user_data,
                                 const void *data);
typedef int    (*keycmp_t)      (void *user_data,
                                 const void *key1,
                                 const void *key2);

typedef void * (*mmalloc_t) (void *user_data, uint32_t n);
typedef void   (*mmfree_t)  (void *user_data, void *ptr);

typedef struct _mmallocator {
    mmalloc_t alloc;
    mmfree_t  free;
    void *    user_data;
} mmallocator_t;

typedef struct _bag {
    bagnode_t **   bucket;           /* Bag nodes */

    mmallocator_t *alk;              /* Memory Allocator */
    bag_hash_t     hash_func;        /* Hash Function */
    keycmp_t       keycmp_func;      /* Key Compare Func */
    mmfree_t       key_free_func;    /* Key Free Func */
    void *         user_data;        /* User Data passed to Key/Value Funcs */

    bagnode_t *    pool;             /* Free nodes pool */
    size_t         pool_size;        /* Free nodes pool size */

    size_t         size;             /* Bag bucket size */
    size_t         used;             /* Bag number of items */
} bag_t;

bag_t *      bag_alloc    (bag_t *bag,
                           size_t size,
                           keycmp_t key_cmp_func,
                           bag_hash_t hash_func,
                           mmallocator_t *allocator,
                           mmfree_t key_free_func,
                           void *user_data);
void         bag_free     (bag_t *bag);

int          bag_clear    (bag_t *bag);

int          bag_insert   (bag_t *bag,
                           void *key);
int          bag_remove   (bag_t *bag,
                           const void *key);

size_t       bag_contains (const bag_t *bag,
                           const void *key);
void *       bag_lookup   (const bag_t *bag,
                           const void *key);

size_t       bag_size     (const bag_t *bag);

void         bag_foreach  (const bag_t *bag,
                           const bag_foreach_t func,
                           void *user_data);

#endif /* !_BAG_H_ */

