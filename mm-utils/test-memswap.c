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
#include "memswap.h"

int main (int argc, char **argv) {
    unsigned long x1[5] = {0, 1, 2, 3, 4};
    unsigned long x2[5] = {5, 6, 7, 8, 9};
    unsigned long a = 10;
    unsigned long b = 256;
    unsigned int i;

    memswap(&a, &b, sizeof(unsigned long));
    printf("A: %lu B: %lu\n", a, b);

    memswap(&x1, &x2, sizeof(x1));
    printf("\nX1: "); for (i = 0; i < 5; ++i) printf("%lu ", x1[i]);
    printf("\nX2: "); for (i = 0; i < 5; ++i) printf("%lu ", x2[i]);
    printf("\n\n");

    memswap8(&a, &b, sizeof(unsigned long));
    printf("A: %lu B: %lu\n", a, b);

    memswap16(&a, &b, sizeof(unsigned long));
    printf("A: %lu B: %lu\n", a, b);

    return(0);
}

