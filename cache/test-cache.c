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

#include <string.h>
#include <stdio.h>
#include "cache.h"

static int __keycmp (void *user_data, const void *k1, const void *k2) {
    return(strcmp((const char *)k1, (const char *)k2));
}

static size_t __hash (void *user_data, const void *key)
{
    size_t hash = 0;
    const char *p;

    for (p = (const char *)key; *p != '\0'; p++)
        hash = (hash << 5) - hash + *p;

    return(hash);
}

int main (int argc, char **argv) {
    cache_t cache;
    void *item;

    cache_alloc(&cache, CACHE_LRU, 5, __keycmp, __hash, NULL, NULL, NULL);
    cache_insert(&cache, "Key1", "Item 1");
    cache_insert(&cache, "Key2", "Item 2");
    cache_insert(&cache, "Key3", "Item 3");
    cache_insert(&cache, "Key4", "Item 4");
    cache_insert(&cache, "Key5", "Item 5");
    cache_insert(&cache, "Key6", "Item 6");
    cache_insert(&cache, "Key7", "Item 7");

    if ((item = cache_retain(&cache, "Key1")) != NULL) {
        printf("Key1 %s\n", (char *)item);
        cache_release(&cache, "Key1");
    }

    if ((item = cache_retain(&cache, "Key2")) != NULL) {
        printf("Key2 %s\n", (char *)item);
        cache_release(&cache, "Key2");
    }

    if ((item = cache_retain(&cache, "Key3")) != NULL) {
        printf("Key3 %s\n", (char *)item);
        cache_release(&cache, "Key3");
    }

    if ((item = cache_retain(&cache, "Key4")) != NULL) {
        printf("Key4 %s\n", (char *)item);

        if ((item = cache_retain(&cache, "Key4")) != NULL)
            printf("--Key4 %s\n", (char *)item);

        cache_release(&cache, "Key4");
        cache_release(&cache, "Key4");
    }

    if ((item = cache_retain(&cache, "Key5")) != NULL) {
        printf("Key5 %s\n", (char *)item);
        cache_release(&cache, "Key5");
    }

    if ((item = cache_retain(&cache, "Key6")) != NULL) {
        printf("Key6 %s\n", (char *)item);
        cache_release(&cache, "Key6");
    }

    if ((item = cache_retain(&cache, "Key7")) != NULL) {
        printf("Key7 %s\n", (char *)item);
        cache_release(&cache, "Key7");
    }

    cache_free(&cache);

    return(0);
}

