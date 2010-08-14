/* [ cache.c ] - LRU/MRU Cache
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

#include <stddef.h>
#include <stdlib.h>
#include "cache.h"

struct _cnode {
    cnode_t *     next;
    cnode_t *     prev;
    cnode_t *     hash;

    const void *  key;
    void *        data;
    size_t        retain;
};

#define __mmalk_data(cache)     ((cache)->alk->user_data)
#define __mmalloc(cache, n)     ((cache)->alk->alloc(__mmalk_data(cache), (n)))
#define __mmfree(cache, ptr)    ((cache)->alk->free(__mmalk_data(cache), (ptr)))

#define __ht_size(n)            ((n) * sizeof(cnode_t *))
#define __ht_alloc(cache, n)    ((cnode_t **) __mmalloc(cache, __ht_size(n)))


/* Default stdlib allocator */
static void *__dmmalloc (void *x, size_t n) { return(malloc(n)); }
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

static cnode_t *__cache_node_alloc (cache_t *cache,
                                    const void *key,
                                    void *data)
{
    cnode_t *node;

    if ((node = __mmalloc(cache, sizeof(cnode_t))) == NULL)
        return(NULL);

    node->key = key;
    node->data = data;
    node->hash = NULL;
    node->retain = 0;

    return(node);
}

static void __cache_node_free (cache_t *cache,
                               cnode_t *node)
{
    if (cache->free_func != NULL)
        cache->free_func(cache->user_data, node->data);

    __mmfree(cache, node);
}

static cnode_t **__cache_ht_lookup (const cache_t *cache,
                                    const void *key)
{
    cnode_t **node;
    size_t hash;

    hash = cache->hash_func(cache->user_data, key);
    node = &(cache->hashtable[hash & cache->mask]);

    while (*node && cache->keycmp_func(cache->user_data, (*node)->key, key))
        node = &((*node)->hash);

    return(node);
}

static cnode_t *__cache_ht_insert (cache_t *cache,
                                   const void *key,
                                   void *data)
{
    cnode_t **node;

    if (*(node = __cache_ht_lookup(cache, key)) != NULL)
        return(NULL);

    if ((*node = __cache_node_alloc(cache, key, data)) == NULL)
        return(NULL);

    cache->used++;

    return(*node);
}

static int __cache_ht_remove (cache_t *cache,
                              cnode_t **node)
{
    cnode_t *p;

    p = *node;
    *node = p->hash;
    __cache_node_free(cache, p);
    cache->used--;

    return(0);
}

static int __cache_node_purge (cache_t *cache)
{
    cnode_t *node;

    switch (cache->type) {
        case CACHE_MRU:
            node = cache->head;
            cache->head = node->next;
            cache->head->prev = NULL;
            break;
        case CACHE_LRU:
            node = cache->tail;
            cache->tail = node->prev;
            cache->tail->next = NULL;
            break;
        default:
            return(-1);
    }

    __cache_ht_remove(cache, &node);
    __cache_node_free(cache, node);

    return(0);
}

cache_t *cache_alloc (cache_t *cache,
                      cache_type_t type,
                      size_t size,
                      item_keycmp_t keycmp_func,
                      item_hash_t hash_func,
                      mmallocator_t *allocator,
                      mmfree_t free_func,
                      void *user_data)
{
    size_t nbucket;

    /* Init Allocator */
    cache->alk = (allocator != NULL) ? allocator : &__default_mmallocator;

    /* Allocate Hashtable */
    nbucket = __nbucket_roundup(size);
    if ((cache->hashtable = __ht_alloc(cache, nbucket)) == NULL)
        return(NULL);

    cache->keycmp_func = keycmp_func;
    cache->hash_func = hash_func;
    cache->free_func = free_func;
    cache->user_data = user_data;

    cache->mask = nbucket - 1;
    cache->size = size;
    cache->used = 0U;
    cache->head = NULL;
    cache->tail = NULL;
    cache->retained = NULL;
    cache->type = type;

    return(cache);
}

int cache_free (cache_t *cache) {
    if (cache_clear(cache))
        return(-1);

    __mmfree(cache, cache->hashtable);
    return(0);
}

int cache_insert (cache_t *cache,
                  const void *key,
                  void *data)
{
    cnode_t *node;

    /* Cache is full, purge one element */
    if (cache->used == cache->size) {
        if (__cache_node_purge(cache))
            return(-1);
    }

    if ((node = __cache_ht_insert(cache, key, data)) == NULL)
        return(-2);

    if (cache->head != NULL)
        cache->head->prev = node;
    node->next = cache->head;
    node->prev = NULL;
    cache->head = node;
    if (cache->tail == NULL)
        cache->tail = node;

    return(0);
}

int cache_remove (cache_t *cache,
                  const void *key)
{
    cnode_t **node;
    cnode_t *p;

    if (*(node = __cache_ht_lookup(cache, key)) == NULL)
        return(-1);

    p = *node;
    if (p->retain > 0)
        return(-2);

    if (p->prev != NULL)
        p->prev->next = p->next;
    if (p->next != NULL)
        p->next->prev = p->prev;

    __cache_ht_remove(cache, node);
    __cache_node_free(cache, p);
    return(0);
}

int cache_clear (cache_t *cache)
{
    cnode_t *node;

    if (cache->retained != NULL)
        return(-1);

    while (cache->head != NULL) {
        node = cache->head;
        cache->head = node->next;
        __cache_ht_remove(cache, &node);
    }

    cache->tail = NULL;

    return(0);
}

int cache_contains (const cache_t *cache,
                    const void *key)
{
    return(*__cache_ht_lookup(cache, key) != NULL);
}

void *cache_retain (cache_t *cache,
                    const void *key)
{
    cnode_t *node;

    if ((node = *__cache_ht_lookup(cache, key)) == NULL)
        return(NULL);

    if (node->retain++ == 0) {
        /* Update Cache List */
        if (node->prev != NULL)
            node->prev->next = node->next;
        if (node->next != NULL)
            node->next->prev = node->prev;

        /* Update Retain List */
        node->prev = NULL;
        node->next = cache->retained;
        cache->retained = node;
    }

    return(node->data);
}

int cache_release (cache_t *cache,
                   const void *key)
{
    cnode_t *node;

    if ((node = *__cache_ht_lookup(cache, key)) == NULL)
        return(-1);

    if (--(node->retain) == 0) {
        /* Update Retain List */
        if (node->prev != NULL)
            node->prev->next = node->next;
        if (node->next != NULL)
            node->next->prev = node->prev;

        /* Update Cache List */
        node->next = cache->head;
        node->prev = NULL;
        cache->head = node;
        if (cache->tail == NULL)
            cache->tail = node;
    }

    return(0);
}

