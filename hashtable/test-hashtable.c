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

#include "hashtable.h"

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

static void __foreach (void *user_data, const void *key, void *value) {
    printf("[%s] = '%s'\n", key, value);
}

int main (int argc, char **argv) {
    hashtable_t table;

    hashtable_alloc(&table, 6, __keycmp, __hash, NULL, NULL, NULL, NULL);
    printf("HT SIZE: %u\n", table.size);
    printf("HT USED: %u\n", table.used);

    hashtable_insert(&table, "Key0", "Value 0");
    hashtable_insert(&table, "Key1", "Value 1");
    hashtable_insert(&table, "Key2", "Value 2");
    hashtable_insert(&table, "Key3", "Value 3");
    hashtable_insert(&table, "Key4", "Value 4");
    hashtable_insert(&table, "Key5", "Value 5");
    hashtable_insert(&table, "Key6", "Value 6");
    hashtable_insert(&table, "Key7", "Value 7");

    printf("HT SIZE: %u\n", table.size);
    printf("HT USED: %u\n", table.used);
    hashtable_foreach(&table, __foreach, NULL);

    hashtable_insert(&table, "Key8", "Value 8");
    hashtable_insert(&table, "Key9", "Value 9");
    hashtable_insert(&table, "Key10", "Value 10");
    hashtable_insert(&table, "Key11", "Value 11");

    printf("HT SIZE: %u\n", table.size);
    printf("HT USED: %u\n", table.used);
    hashtable_foreach(&table, __foreach, NULL);

    hashtable_remove(&table, "Key2");
    printf("HT SIZE: %u\n", table.size);
    printf("HT USED: %u\n", table.used);
    hashtable_foreach(&table, __foreach, NULL);


    hashtable_free(&table);

    return(0);
}

