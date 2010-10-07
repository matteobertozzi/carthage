/* [ set.c ] - Set
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

#include "set.h"

#define __SET_POOL_SIZE   (32)

#define __mmalk_data(set)    ((set)->alk->user_data)
#define __mmalloc(set, n)    ((set)->alk->alloc(__mmalk_data(set), (n)))
#define __mmfree(set, ptr)   ((set)->alk->free(__mmalk_data(set), (ptr)))

#define __bucket_size(n)       (sizeof(setnode_t) * (n))
#define __bucket_alloc(t, n)   ((setnode_t **) __mmalloc(t, __bucket_size(n)))

struct _set_node {
    setnode_t *next;
    void *     key;
};

/* Default stdlib allocator */
static void *__dmmalloc (void *x, uint32_t n) { return(malloc(n)); }
static void __dmmfree (void *x, void *ptr) { free(ptr); }
static mmallocator_t __default_mmallocator = {
    .alloc = __dmmalloc,
    .free  = __dmmfree,
    .user_data = NULL,
};

static size_t __nbucket_roundup (size_t size) {
    if (size < 8U)
        return(8U);

    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
#if defined(__LP64__)
    size |= size >> 32;
#endif
    size++;
    return(size);
}

static setnode_t *__setnode_alloc (set_t *set,
                                   void *key)
{
    setnode_t *node;

    if (set->pool != NULL) {
        node = set->pool;
        set->pool = node->next;
        set->pool_size--;
    } else {
        if ((node = (setnode_t *)__mmalloc(set, sizeof(setnode_t))) == NULL)
            return(NULL);
    }

    node->key = key;
    node->next = NULL;

    return(node);
}

static void __setnode_free (set_t *set,
                            setnode_t *node)
{
    if (set->key_free_func != NULL)
        set->key_free_func(set->user_data, node->key);

    if (set->pool_size < __SET_POOL_SIZE) {
        node->next = set->pool;
        set->pool = node;
        set->pool_size++;
    } else {
        __mmfree(set, node);
    }
}

static setnode_t **__setnode_lookup (const set_t *set,
                                     const void *key)
{
    setnode_t **node;
    size_t hash;

    hash = set->hash_func(set->user_data, key);

    node = &(set->bucket[hash & (set->size - 1)]);

    while (*node && set->keycmp_func(set->user_data, (*node)->key, key))
        node = &((*node)->next);

    return(node);
}

static int __set_resize (set_t *set,
                         size_t new_size)
{
    setnode_t **bucket;
    setnode_t *next;
    setnode_t *p;
    size_t used;
    size_t size;

    /* Round up to size a power of two */
    new_size = __nbucket_roundup(new_size);

    /* Store old bucket and sizes */
    size = set->size;
    used = set->used;
    bucket = set->bucket;

    /* Allocate new bucket */
    set->size = new_size;
    if ((set->bucket = __bucket_alloc(set, new_size)) == NULL)
        return(-1);

    /* Fill new bucket with old nodes */
    memset(set->bucket, 0, __bucket_size(new_size));
    while (used--) {
        for (p = bucket[used]; p != NULL; p = next) {
            next = p->next;

            *__setnode_lookup(set, p->key) = p;
            p->next = NULL;
        }
    }

    __mmfree(set, bucket);

    return(0);
}

set_t *set_alloc (set_t *set,
                  size_t size,
                  keycmp_t key_cmp_func,
                  set_hash_t hash_func,
                  mmallocator_t *allocator,
                  mmfree_t key_free_func,
                  void *user_data)
{
    /* Round up to size a power of two */
    size = __nbucket_roundup(size);

    /* Init Allocator */
    set->alk = (allocator != NULL) ? allocator : &__default_mmallocator;

    /* Allocate Bucket */
    if ((set->bucket = __bucket_alloc(set, size)) == NULL)
        return(NULL);

    memset(set->bucket, 0, __bucket_size(size));
    set->used = 0U;
    set->size = size;

    set->pool = NULL;
    set->pool_size = 0U;

    set->user_data = user_data;
    set->hash_func = hash_func;
    set->keycmp_func = key_cmp_func;
    set->key_free_func = key_free_func;

    return(set);
}

void set_free (set_t *set) {
    setnode_t *next;
    setnode_t *p;

    set_clear(set);
    for (p = set->pool; p != NULL; p = next) {
        next = p->next;
        __mmfree(set, p);
    }

    __mmfree(set, set->bucket);
}

int set_insert (set_t *set,
                void *key)
{
    setnode_t **node;

    /* Lookup Node, do nothing, value already in */
    if (*(node = __setnode_lookup(set, key)) != NULL)
        return(0);

    /* Resize Table if necessary */
    if (set->used > (set->size + (set->size >> 3))) {
        unsigned long size = set->size;

        size += (size < 64) ? (size >> 1) : (size);
        if (__set_resize(set, size))
            return(-1);

        node = __setnode_lookup(set, key);
    }

    /* Allocate new node for this entry */
    if ((*node = __setnode_alloc(set, key)) == NULL)
        return(-2);

    set->used++;

    return(0);
}

int set_remove (set_t *set,
                const void *key)
{
    setnode_t **node;
    setnode_t *p;

    if (*(node = __setnode_lookup(set, key)) == NULL)
        return(-1);

    /* Remove Item */
    p = *node;
    *node = p->next;
    __setnode_free(set, p);
    set->used--;

    /* Resize back if value is used is small */
    if (set->used < ((set->size >> 1) - (set->size >> 2)))
        __set_resize(set, set->size >> 1);

    return(0);
}

int set_clear (set_t *set)
{
    setnode_t *next;
    setnode_t *p;
    size_t size;

    size = set->size;
    while (size--) {
        for (p = set->bucket[size]; p != NULL; p = next) {
            next = p->next;
            __setnode_free(set, p);
        }
        set->bucket[size] = NULL;
    }

    set->used = 0;

    /* Resize back if value is used is small */
    if (set->size > 1024)
        __set_resize(set, set->size >> 3);

    return(0);
}

int set_contains (const set_t *set,
                  const void *key)
{
    return(*__setnode_lookup(set, key) != NULL);
}

void *set_lookup (const set_t *set,
                  const void *key)
{
    setnode_t *node = *__setnode_lookup(set, key);
    return(node != NULL ? node->key : NULL);
}

size_t set_size (const set_t *set)
{
    return(set->used);
}

void set_foreach (const set_t *set,
                        const set_foreach_t func,
                        void *user_data)
{
    setnode_t *p;
    size_t size;

    size = set->size;
    while (size--) {
        for (p = set->bucket[size]; p != NULL; p = p->next)
            func(user_data, p->key);
    }
}

