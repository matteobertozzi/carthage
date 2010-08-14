/* [ cache.h ] - LRU/MRU Cache
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

#ifndef _CACHE_H_
#define _CACHE_H_

typedef struct _cnode cnode_t;

typedef size_t (*item_hash_t)   (void *user_data,
                                 const void *data);
typedef int    (*item_keycmp_t) (void *user_data,
                                 const void *key1,
                                 const void *key2);

typedef void * (*mmalloc_t) (void *user_data, size_t n);
typedef void   (*mmfree_t)  (void *user_data, void *ptr);

typedef struct _mmallocator {
    mmalloc_t alloc;
    mmfree_t  free;
    void *    user_data;
} mmallocator_t;

typedef enum _cache_type {
    CACHE_LRU,
    CACHE_MRU,
} cache_type_t;

typedef struct _cache {
    mmallocator_t *alk;

    item_keycmp_t keycmp_func;
    item_hash_t   hash_func;
    mmfree_t      free_func;
    void *        user_data;

    cnode_t *     head;
    cnode_t *     tail;
    cnode_t *     retained;
    cnode_t **    hashtable;

    size_t        used;
    size_t        size;
    size_t        mask;
    cache_type_t  type;
} cache_t;

cache_t *   cache_alloc         (cache_t *cache,
                                 cache_type_t type,
                                 size_t size,
                                 item_keycmp_t keycmp_func,
                                 item_hash_t hash_func,
                                 mmallocator_t *allocator,
                                 mmfree_t free_func,
                                 void *user_data);
int         cache_free          (cache_t *cache);

int         cache_insert        (cache_t *cache,
                                 const void *key,
                                 void *data);
int         cache_remove        (cache_t *cache,
                                 const void *key);
int         cache_clear         (cache_t *cache);

int         cache_contains      (const cache_t *cache,
                                 const void *key);
void *      cache_retain        (cache_t *cache,
                                 const void *key);
int         cache_release       (cache_t *cache,
                                 const void *key);

#endif /* !_CACHE_H_ */

