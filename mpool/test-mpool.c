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
#include "mpool.h"

int main (int argc, char **argv) {
    uint8_t buffer[1024];
    void *pointers[4];
    mpool_t mpool;
    int i;

    mpool_init(&mpool, buffer, sizeof(buffer));
    printf("Free %u\n", mpool.free);

    for (i = 0; i < 4; ++i) {
        pointers[i] = mpool_alloc(&mpool, 245);
        printf("%d: %p - Free %u\n", i, pointers[i], mpool.free);
    }

    for (i = 0; i < 4; ++i)
        mpool_free(&mpool, pointers[i]);

    pointers[0] = mpool_alloc(&mpool, 512);
    printf("0: %ld\n", (void *)pointers[0] - (void *)buffer);

    pointers[1] = mpool_alloc(&mpool, 512);
    printf("1: IS NULL %d\n", pointers[1] == NULL);

    mpool_free(&mpool, pointers[0]);

    printf("Free %u %u\n", mpool.free, *((uint32_t *)buffer));

    pointers[0] = NULL;
    for (i = 0; i < 9; ++i) {
        pointers[0] = mpool_realloc(&mpool, pointers[0], (i + 1) << 1);

        pointers[1] = mpool_alloc(&mpool, 12);

        printf("0: %ld (%ld) %u\n", (pointers[1] - (void *)buffer), (pointers[0] - (void *)buffer), mpool.free);
        mpool_free(&mpool, pointers[1]);
    }

    for (i = 9; i >= 0; --i) {
        pointers[0] = mpool_realloc(&mpool, pointers[0], (i + 1) << 1);

        pointers[1] = mpool_alloc(&mpool, 12);
        printf("0: %ld (%ld) %u\n", (pointers[1] - (void *)buffer), (pointers[0] - (void *)buffer), mpool.free);
        mpool_free(&mpool, pointers[1]);
    }

    mpool_free(&mpool, pointers[0]);
    printf("Free %u\n", mpool.free);

    return(0);
}

