/* [ bag.c ] - Bag
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
#include <stdlib.h>

#include "bag.h"

#define __BAG_POOL_SIZE   (32)

#define __mmalk_data(bag)    ((bag)->alk->user_data)
#define __mmalloc(bag, n)    ((bag)->alk->alloc(__mmalk_data(bag), (n)))
#define __mmfree(bag, ptr)   ((bag)->alk->free(__mmalk_data(bag), (ptr)))

#define __bucket_size(n)       (sizeof(bagnode_t) * (n))
#define __bucket_alloc(t, n)   ((bagnode_t **) __mmalloc(t, __bucket_size(n)))

struct _bag_node {
    bagnode_t *next;
    void *     key;
    size_t     count;
};

/* Default stdlib allocator */
static void *__dmmalloc (void *x, uint32_t n) { return(malloc(n)); }
static void __dmmfree (void *x, void *ptr) { free(ptr); }
static mmallocator_t __default_mmallocator = {
    .alloc = __dmmalloc,
    .free  = __dmmfree,
    .user_data = NULL,
};

static bagnode_t *__bagnode_alloc (bag_t *bag,
                                   void *key)
{
    bagnode_t *node;

    if (bag->pool != NULL) {
        node = bag->pool;
        bag->pool = node->next;
        bag->pool_size--;
    } else {
        if ((node = (bagnode_t *)__mmalloc(bag, sizeof(bagnode_t))) == NULL)
            return(NULL);
    }

    node->key = key;
    node->count = 1U;
    node->next = NULL;

    return(node);
}

static void __bagnode_free (bag_t *bag,
                            bagnode_t *node)
{
    if (bag->key_free_func != NULL)
        bag->key_free_func(bag->user_data, node->key);

    if (bag->pool_size < __BAG_POOL_SIZE) {
        node->next = bag->pool;
        bag->pool = node;
        bag->pool_size++;
    } else {
        __mmfree(bag, node);
    }
}

static bagnode_t **__bagnode_lookup (const bag_t *bag,
                                     const void *key)
{
    bagnode_t **node;
    size_t hash;

    hash = bag->hash_func(bag->user_data, key);

    node = &(bag->bucket[hash & (bag->size - 1)]);

    while (*node && bag->keycmp_func(bag->user_data, (*node)->key, key))
        node = &((*node)->next);

    return(node);
}

static int __bag_resize (bag_t *bag,
                         size_t new_size)
{
    bagnode_t **bucket;
    bagnode_t *next;
    bagnode_t *p;
    size_t used;
    size_t size;

    /* Round up to size a power of two */
    size = new_size;
    for (new_size = 8; new_size < size; new_size <<= 1)
        continue;

    /* Store old bucket and sizes */
    size = bag->size;
    used = bag->used;
    bucket = bag->bucket;

    /* Allocate new bucket */
    bag->size = new_size;
    if ((bag->bucket = __bucket_alloc(bag, new_size)) == NULL)
        return(-1);

    /* Fill new bucket with old nodes */
    memset(bag->bucket, 0, __bucket_size(size));
    while (used--) {
        for (p = bucket[used]; p != NULL; p = next) {
            next = p->next;

            *__bagnode_lookup(bag, p->key) = p;
            p->next = NULL;
        }
    }

    __mmfree(bag, bucket);

    return(0);
}

bag_t *bag_alloc (bag_t *bag,
                  size_t size,
                  keycmp_t key_cmp_func,
                  bag_hash_t hash_func,
                  mmallocator_t *allocator,
                  mmfree_t key_free_func,
                  void *user_data)
{
    size_t real;

    /* Round up to size a power of two */
    for (real = 8; real < size; real <<= 1)
        continue;

    /* Init Allocator */
    bag->alk = (allocator != NULL) ? allocator : &__default_mmallocator;

    /* Allocate Bucket */
    if ((bag->bucket = __bucket_alloc(bag, real)) == NULL)
        return(NULL);

    memset(bag->bucket, 0, __bucket_size(size));
    bag->used = 0U;
    bag->size = real;

    bag->pool = NULL;
    bag->pool_size = 0U;

    bag->user_data = user_data;
    bag->hash_func = hash_func;
    bag->keycmp_func = key_cmp_func;
    bag->key_free_func = key_free_func;

    return(bag);
}

void bag_free (bag_t *bag) {
    bagnode_t *next;
    bagnode_t *p;

    bag_clear(bag);
    for (p = bag->pool; p != NULL; p = next) {
        next = p->next;
        __mmfree(bag, p);
    }

    __mmfree(bag, bag->bucket);
}

int bag_insert (bag_t *bag,
                void *key)
{
    bagnode_t **node;

    /* Lookup Node, if not null Increment Value Count */
    if (*(node = __bagnode_lookup(bag, key)) != NULL) {
        (*node)->count++;
        return(0);
    }

    /* Resize Table if necessary */
    if (bag->used > (bag->size + (bag->size >> 3))) {
        unsigned long size = bag->size;

        size += (size < 64) ? (size >> 1) : (size);
        if (__bag_resize(bag, size))
            return(-1);
    }

    /* Allocate new node for this entry */
    if ((*node = __bagnode_alloc(bag, key)) == NULL)
        return(-2);

    bag->used++;

    return(0);
}

int bag_remove (bag_t *bag,
                const void *key)
{
    bagnode_t **node;
    bagnode_t *p;

    if (*(node = __bagnode_lookup(bag, key)) == NULL)
        return(-1);

    /* Remove Number of occurrences */
    p = *node;
    p->count--;

    /* Remove Item */
    if (p->count == 0) {
        *node = p->next;
        __bagnode_free(bag, p);
        bag->used--;

        /* Resize back if value is used is small */
        if (bag->used < ((bag->size >> 1) - (bag->size >> 2)))
            __bag_resize(bag, bag->size >> 1);
    }

    return(0);
}

int bag_clear (bag_t *bag)
{
    bagnode_t *next;
    bagnode_t *p;
    size_t size;

    size = bag->size;
    while (size--) {
        for (p = bag->bucket[size]; p != NULL; p = next) {
            next = p->next;
            __bagnode_free(bag, p);
        }
        bag->bucket[size] = NULL;
    }

    bag->used = 0;

    /* Resize back if value is used is small */
    if (bag->size > 1024)
        __bag_resize(bag, bag->size >> 3);

    return(0);
}

size_t bag_contains (const bag_t *bag,
                     const void *key)
{
    bagnode_t *node = *__bagnode_lookup(bag, key);
    return(node != NULL ? node->count : 0U);
}

void *bag_lookup (const bag_t *bag,
                  const void *key)
{
    bagnode_t *node = *__bagnode_lookup(bag, key);
    return(node != NULL ? node->key : NULL);
}

size_t bag_size (const bag_t *bag)
{
    return(bag->used);
}

void bag_foreach (const bag_t *bag,
                        const bag_foreach_t func,
                        void *user_data)
{
    bagnode_t *next;
    bagnode_t *p;
    size_t size;

    size = bag->size;
    while (size--) {
        for (p = bag->bucket[size]; p != NULL; p = p->next)
            func(user_data, p->key, p->count);
    }
}

