/* [ hashtable.c ] - Hash Table
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

#include "hashtable.h"

#define __HASHTABLE_POOL_SIZE   (32)

#define __mmalk_data(table)    ((table)->alk->user_data)
#define __mmalloc(table, n)    ((table)->alk->alloc(__mmalk_data(table), (n)))
#define __mmfree(table, ptr)   ((table)->alk->free(__mmalk_data(table), (ptr)))

#define __bucket_size(n)       (sizeof(hashnode_t) * (n))
#define __bucket_alloc(t, n)   ((hashnode_t **) __mmalloc(t, __bucket_size(n)))

struct _hash_node {
    hashnode_t * next;
    void *        key;
    void *        value;
};

/* Default stdlib allocator */
static void *__dmmalloc (void *x, uint32_t n) { return(malloc(n)); }
static void __dmmfree (void *x, void *ptr) { free(ptr); }
static mmallocator_t __default_mmallocator = {
    .alloc = __dmmalloc,
    .free  = __dmmfree,
    .user_data = NULL,
};

static hashnode_t *__hashnode_alloc (hashtable_t *table,
                                     void *key,
                                     void *value)
{
    hashnode_t *node;

    if (table->pool != NULL) {
        node = table->pool;
        table->pool = node->next;
        table->pool_size--;
    } else {
        if ((node = (hashnode_t *)__mmalloc(table, sizeof(hashnode_t))) == NULL)
            return(NULL);
    }

    node->key = key;
    node->value = value;
    node->next = NULL;

    return(node);
}

static void __hashnode_free (hashtable_t *table,
                             hashnode_t *node)
{
    if (table->key_free_func != NULL)
        table->key_free_func(table->user_data, node->key);

    if (table->value_free_func != NULL)
        table->value_free_func(table->user_data, node->value);

    if (table->pool_size < __HASHTABLE_POOL_SIZE) {
        node->next = table->pool;
        table->pool = node;
        table->pool_size++;
    } else {
        __mmfree(table, node);
    }
}

static hashnode_t **__hashnode_lookup (const hashtable_t *table,
                                       const void *key)
{
    hashnode_t **node;
    size_t hash;

    hash = table->hash_func(table->user_data, key);

    node = &(table->bucket[hash & (table->size - 1)]);

    while (*node && table->keycmp_func(table->user_data, (*node)->key, key))
        node = &((*node)->next);

    return(node);
}

static int __hashtable_resize (hashtable_t *table,
                               size_t new_size)
{
    hashnode_t **bucket;
    hashnode_t *next;
    hashnode_t *p;
    size_t used;
    size_t size;

    /* Round up to size a power of two */
    size = new_size;
    for (new_size = 8; new_size < size; new_size <<= 1)
        continue;

    /* Store old bucket and sizes */
    size = table->size;
    used = table->used;
    bucket = table->bucket;

    /* Allocate new bucket */
    table->size = new_size;
    if ((table->bucket = __bucket_alloc(table, new_size)) == NULL)
        return(-1);

    /* Fill new bucket with old nodes */
    memset(table->bucket, 0, __bucket_size(size));
    while (used--) {
        for (p = bucket[used]; p != NULL; p = next) {
            next = p->next;

            *__hashnode_lookup(table, p->key) = p;
            p->next = NULL;
        }
    }

    __mmfree(table, bucket);

    return(0);
}

hashtable_t *hashtable_alloc (hashtable_t *table,
                              size_t size,
                              keycmp_t key_cmp_func,
                              hashtable_hash_t hash_func,
                              mmallocator_t *allocator,
                              mmfree_t key_free_func,
                              mmfree_t value_free_func,
                              void *user_data)
{
    size_t real;

    /* Round up to size a power of two */
    for (real = 8; real < size; real <<= 1)
        continue;

    /* Init Allocator */
    table->alk = (allocator != NULL) ? allocator : &__default_mmallocator;

    /* Allocate Bucket */
    if ((table->bucket = __bucket_alloc(table, real)) == NULL)
        return(NULL);

    memset(table->bucket, 0, __bucket_size(size));
    table->used = 0U;
    table->size = real;

    table->pool = NULL;
    table->pool_size = 0U;

    table->user_data = user_data;
    table->hash_func = hash_func;
    table->keycmp_func = key_cmp_func;
    table->key_free_func = key_free_func;
    table->value_free_func = value_free_func;

    return(table);
}

void hashtable_free (hashtable_t *table) {
    hashnode_t *next;
    hashnode_t *p;

    hashtable_clear(table);
    for (p = table->pool; p != NULL; p = next) {
        next = p->next;
        __mmfree(table, p);
    }

    __mmfree(table, table->bucket);
}

int hashtable_insert (hashtable_t *table,
                      void *key,
                      void *value)
{
    hashnode_t **node;

    /* Lookup Node, if not null Replace old value with the new value */
    if (*(node = __hashnode_lookup(table, key)) != NULL) {
        if ((*node)->value != value && table->value_free_func != NULL)
            table->value_free_func(table->user_data, (*node)->value);
        (*node)->value = value;
        return(0);
    }

    /* Resize Table if necessary */
    if (table->used > (table->size + (table->size >> 3))) {
        unsigned long size = table->size;

        size += (size < 64) ? (size >> 1) : (size);
        if (__hashtable_resize(table, size))
            return(-1);
    }

    /* Allocate new node for this entry */
    if ((*node = __hashnode_alloc(table, key, value)) == NULL)
        return(-2);

    table->used++;

    return(0);
}

int hashtable_remove (hashtable_t *table,
                      const void *key)
{
    hashnode_t **node;
    hashnode_t *p;

    if (*(node = __hashnode_lookup(table, key)) == NULL)
        return(-1);

    /* Remove Item */
    p = *node;
    *node = p->next;
    __hashnode_free(table, p);
    table->used--;

    /* Resize back if value is used is small */
    if (table->used < ((table->size >> 1) - (table->size >> 2)))
        __hashtable_resize(table, table->size >> 1);

    return(0);
}

int hashtable_clear (hashtable_t *table)
{
    hashnode_t *next;
    hashnode_t *p;
    size_t size;

    size = table->size;
    while (size--) {
        for (p = table->bucket[size]; p != NULL; p = next) {
            next = p->next;
            __hashnode_free(table, p);
        }
        table->bucket[size] = NULL;
    }

    table->used = 0;

    /* Resize back if value is used is small */
    if (table->size > 1024)
        __hashtable_resize(table, table->size >> 3);

    return(0);
}

int hashtable_contains (const hashtable_t *table,
                        const void *key)
{
    return(*__hashnode_lookup(table, key) != NULL);
}

void *hashtable_lookup (const hashtable_t *table,
                        const void *key)
{
    hashnode_t *node = *__hashnode_lookup(table, key);
    return(node != NULL ? node->value : NULL);
}

size_t hashtable_size (const hashtable_t *table)
{
    return(table->used);
}

void hashtable_foreach (const hashtable_t *table,
                        const hashtable_foreach_t func,
                        void *user_data)
{
    hashnode_t *next;
    hashnode_t *p;
    size_t size;

    size = table->size;
    while (size--) {
        for (p = table->bucket[size]; p != NULL; p = p->next)
            func(user_data, p->key, p->value);
    }
}

