/*
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

#include "set.h"

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

static void __foreach (void *user_data, const void *key) {
    printf("[%s]\n", key);
}

int main (int argc, char **argv) {
    set_t set;

    set_alloc(&set, 6, __keycmp, __hash, NULL, NULL, NULL);
    printf("HT SIZE: %u\n", set.size);
    printf("HT USED: %u\n", set.used);

    set_insert(&set, "Key0");
    set_insert(&set, "Key1");
    set_insert(&set, "Key2");
    set_insert(&set, "Key3");
    set_insert(&set, "Key4");
    set_insert(&set, "Key0");
    set_insert(&set, "Key1");
    set_insert(&set, "Key0");

    printf("HT SIZE: %u\n", set.size);
    printf("HT USED: %u\n", set.used);
    set_foreach(&set, __foreach, NULL);

    set_insert(&set, "Key8");
    set_insert(&set, "Key0");
    set_insert(&set, "Key4");
    set_insert(&set, "Key2");

    printf("HT SIZE: %u\n", set.size);
    printf("HT USED: %u\n", set.used);
    set_foreach(&set, __foreach, NULL);

    set_remove(&set, "Key0");
    printf("HT SIZE: %u\n", set.size);
    printf("HT USED: %u\n", set.used);
    set_foreach(&set, __foreach, NULL);

    set_free(&set);

    return(0);
}

