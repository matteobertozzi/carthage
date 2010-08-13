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

#include "merge.h"

static int __cmpfunc (void *user_data, const void *a, const void *b, size_t size) {
    return(*((const int *)a) - *((const int *)b));
}

static void __cpyfunc (void *user_data, void *src, const void *dst, size_t size) {
    const int *idst = (const int *)dst;
    int *isrc = (int *)src;

    *isrc = *idst;
}

static void __test1 (void) {
    int v1[3] = {1, 3, 5};
    int v2[5] = {2, 4, 6, 7, 8};
    int r[8];
    int i;

    merge(r, 8, v1, 3, v2, 5, sizeof(int), __cmpfunc, __cpyfunc, NULL);

    for (i = 0; i < 8; ++i)
        printf("%d ", r[i]);
    printf("\n");
}

static void __test2 (void) {
    int v1[3] = {1, 3, 7};
    int v2[5] = {2, 4, 5, 6, 8};
    int r[8];
    int i;

    mmmerge(r, 8, v1, 3, v2, 5, sizeof(int));

    for (i = 0; i < 8; ++i)
        printf("%d ", r[i]);
    printf("\n");
}

int main (int argc, char **argv) {
    __test1();
    __test2();
    return(0);
}

