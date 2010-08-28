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

#include <stdlib.h>
#include <stdio.h>

#include "chunkq.h"

int main (int argc, char **argv) {
    char buffer[16];
    chunkq_t chunk;
    ssize_t n;
    size_t i;

    chunkq_alloc(&chunk, 4);

    n = chunkq_append(&chunk, "Hello", 5);
    printf("Append: %u %u\n", chunk.size, n);

    n = chunkq_append(&chunk, " ABCDEFGHKILMNOPQRSTUVWXZ", 25);
    printf("Append: %u %u\n", chunk.size, n);

    for (i = 0; i < 40; ++i) {
        if ((n = chunkq_peek(&chunk, i, buffer, 10)) > 0) {
            buffer[n] = '\0';
            printf("PEEK %u: %d '%s'\n", i, n, buffer);
        }
    }

    if ((n = chunkq_read(&chunk, buffer, 2)) > 0) {
        buffer[n] = '\0';
        printf("READ %u: %s (%u)\n", n, buffer, chunk.size);
    }

    if ((n = chunkq_peek(&chunk, 0, buffer, 10)) > 0) {
        buffer[n] = '\0';
        printf("PEEK %u: %d '%s'\n", i, n, buffer);
    }

    while ((n = chunkq_read(&chunk, buffer, 15)) > 0) {
        buffer[n] = '\0';
        printf("READ %u: %s (%u)\n", n, buffer, chunk.size);
    }
    printf("\n");

    chunkq_free(&chunk);

    return(0);
}

