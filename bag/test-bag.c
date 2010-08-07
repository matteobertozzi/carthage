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

#include "bag.h"

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

static void __foreach (void *user_data, const void *key, size_t count) {
    printf("[%s] = '%u'\n", key, count);
}

int main (int argc, char **argv) {
    bag_t bag;

    bag_alloc(&bag, 6, __keycmp, __hash, NULL, NULL, NULL);
    printf("HT SIZE: %u\n", bag.size);
    printf("HT USED: %u\n", bag.used);

    bag_insert(&bag, "Key0");
    bag_insert(&bag, "Key1");
    bag_insert(&bag, "Key2");
    bag_insert(&bag, "Key3");
    bag_insert(&bag, "Key4");
    bag_insert(&bag, "Key0");
    bag_insert(&bag, "Key1");
    bag_insert(&bag, "Key0");

    printf("HT SIZE: %u\n", bag.size);
    printf("HT USED: %u\n", bag.used);
    bag_foreach(&bag, __foreach, NULL);

    bag_insert(&bag, "Key8");
    bag_insert(&bag, "Key0");
    bag_insert(&bag, "Key4");
    bag_insert(&bag, "Key2");

    printf("HT SIZE: %u\n", bag.size);
    printf("HT USED: %u\n", bag.used);
    bag_foreach(&bag, __foreach, NULL);

    bag_remove(&bag, "Key0");
    printf("HT SIZE: %u\n", bag.size);
    printf("HT USED: %u\n", bag.used);
    bag_foreach(&bag, __foreach, NULL);

    bag_free(&bag);

    return(0);
}

