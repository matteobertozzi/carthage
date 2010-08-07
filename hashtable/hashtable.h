/* [ hashtable.h ] - Hash Table
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

#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stddef.h>
#include <stdint.h>

typedef struct _hash_node hashnode_t;

typedef void   (*hashtable_foreach_t) (void *user_data,
                                       const void *key,
                                       void *value);
typedef size_t (*hashtable_hash_t)    (void *user_data,
                                       const void *data);
typedef int    (*keycmp_t)            (void *user_data,
                                       const void *key1,
                                       const void *key2);

typedef void * (*mmalloc_t) (void *user_data, uint32_t n);
typedef void   (*mmfree_t)  (void *user_data, void *ptr);

typedef struct _mmallocator {
    mmalloc_t alloc;
    mmfree_t  free;
    void *    user_data;
} mmallocator_t;

typedef struct _hashtable {
    hashnode_t **  bucket;             /* Hashtable nodes */

    mmallocator_t *alk;                /* Memory Allocator */
    hashtable_hash_t hash_func;        /* Hash Function */
    keycmp_t         keycmp_func;      /* Key Compare Func */
    mmfree_t         key_free_func;    /* Key Free Func */
    mmfree_t         value_free_func;  /* Value Free Func */
    void *           user_data;        /* User Data passed to Key/Value Funcs */

    hashnode_t *     pool;             /* Free nodes pool */
    size_t           pool_size;        /* Free nodes pool size */

    size_t           size;             /* Hashtable bucket size */
    size_t           used;             /* Hashtable number of items */
} hashtable_t;

hashtable_t *hashtable_alloc    (hashtable_t *table,
                                 size_t size,
                                 keycmp_t key_cmp_func,
                                 hashtable_hash_t hash_func,
                                 mmallocator_t *allocator,
                                 mmfree_t key_free_func,
                                 mmfree_t value_free_func);
void         hashtable_free     (hashtable_t *table);

int          hashtable_clear    (hashtable_t *table);

int          hashtable_insert   (hashtable_t *table,
                                 void *key,
                                 void *value);
int          hashtable_remove   (hashtable_t *table,
                                 const void *key);

int          hashtable_contains (const hashtable_t *table,
                                 const void *key);
void *       hashtable_lookup   (const hashtable_t *table,
                                 const void *key);

size_t       hashtable_size     (const hashtable_t *table);

void         hashtable_foreach  (const hashtable_t *table,
                                 const hashtable_foreach_t func,
                                 void *user_data);

#endif /* !_HASHTABLE_H_ */

