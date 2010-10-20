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

#include <stdio.h>

#include "bloom.h"

static uint32_t __hash (const void *data, uint32_t seed) {
    const char *p = (const char *)data;
    uint32_t h = seed;

    while (*p != '\0') {
        h  = (h << 4) + *p++;
        h ^= (h & 0xf0000000) >> 23;
        h &= 0x0fffffff;
    }

    return(h);
}

static int __bloom_hash (const void *key,
                         uint32_t *hashes,
                         unsigned int nhashes,
                         unsigned int max_value)
{
    unsigned int seed;
    unsigned int i;

    seed = 0;
    for (i = 0; i < nhashes; ++i) {
        seed =  __hash(key, seed);
        hashes[i] = seed % max_value;
    }

    return(0);
}

int main (int argc, char **argv) {
    const char **p;
    bloom_t bloom;

    const char *tests_in[] = {
        "66269d0fda485a1bd32801fc6ec4f53eea2faa08",
        "508a6ccc545b32641fe7311048defe7cf599ada3",
        "bdd1dbcaab4ea943b8cc1f006823b733762797c4",
        "c98792fc50d7a7d071f1e6e76c0f5b09c778e886",
        "40f1bf0bc332accf1801abfbb5c3367c3b965a8d",
        "7dfc52e314ed9d727e89ccd660022fc55e58e5a3",
        "a052f59393c1867ec35f68f676f45c64ab4fad5c",
        "e526f520264924744a76aa2a73e1325ac45083f0",
        "25fd869f62343eb80bdc9a7ed51ef7a040b35de1",
        "c26e349836949d27709ba89aa16a0f9ae285acdc",
        NULL,
    };
    const char *tests_out[] = {
        "407e27870295e8c497cb2a1f7f16cdce5ca6c6d8",
        "96757c4fbcc982909147deab8d85586d0cc5ea68",
        "d87eca86e6d7777b7b53251024e7b793de9296e1",
        "03f3631977d39c2ca6f9b2c377554f4adc801e20",
        "4054f03020b82160bef3e9ab7d3efdebd06dde49",
        "df1c255cdf73a58f81b4e5e32f48b9fdd8945d36",
        "a4f761a60e9af58dc4922d1dc5dec296deab0008",
        "35a0c8e2c6cb6814f506b64fea0864c0290fd790",
        "deb852b7ea56566a7a8988ee47f37f2e4e63c770",
        NULL,
    };

    if (bloom_alloc(&bloom, 20U, 2U, 50U, __bloom_hash)) {
        printf("bloom_alloc(): Failed\n");
        return(1);
    }

    for (p = tests_in; *p != NULL; ++p)
        bloom_add(&bloom, *p);

    printf("Tests In\n");
    for (p = tests_in; *p != NULL; ++p)
        printf("Contains: %s %d\n", *p, bloom_contains(&bloom, *p));

    printf("Tests Out\n");
    for (p = tests_out; *p != NULL; ++p)
        printf("Contains: %s %d\n", *p, bloom_contains(&bloom, *p));

    bloom_free(&bloom);

    return(0);
}

