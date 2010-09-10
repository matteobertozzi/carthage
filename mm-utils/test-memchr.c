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
#include "memchr.h"

int main (int argc, char **argv) {
    char buf[] = "Hello World This is a Tesht";
    void *p[5];

    p[0] = memchr(buf, 'h', 27);
    p[1] = memchr8(buf, 'h', 27);
    p[2] = memchr16(buf, 'h', 27);
    p[3] = memchr32(buf, 'h', 27);
    p[4] = memchr64(buf, 'h', 27);

    if (!(p[0] == p[1] && p[0] == p[2] && p[0] == p[3] && p[0] == p[4]))
        printf("FAILED h\n");
    printf("%s\n", p[0]);

    p[0] = memchr(buf+1, 'W', 27-1);
    p[1] = memchr8(buf+1, 'W', 27-1);
    p[2] = memchr16(buf+1, 'W', 27-1);
    p[3] = memchr32(buf+1, 'W', 27-1);
    p[4] = memchr64(buf+1, 'W', 27-1);

    if (!(p[0] == p[1] && p[0] == p[2] && p[0] == p[3] && p[0] == p[4]))
        printf("FAILED W\n");
    printf("%s\n", p[0]);


    p[0] = memchr(buf, 'k', 27);
    p[1] = memchr8(buf, 'k', 27);
    p[2] = memchr16(buf, 'k', 27);
    p[3] = memchr32(buf, 'k', 27);
    p[4] = memchr64(buf, 'k', 27);

    if (!(p[0] == p[1] && p[0] == p[2] && p[0] == p[3] && p[0] == p[4]))
        printf("FAILED k\n");

    printf("REVERSED\n");

    p[0] = memrchr(buf, 'h', 27);
    p[1] = memrchr8(buf, 'h', 27);
    p[2] = memrchr16(buf, 'h', 27);
    p[3] = memrchr32(buf, 'h', 27);
    p[4] = memrchr64(buf, 'h', 27);

    printf("%p\n", p[0]);
    printf("%p\n", p[1]);
    printf("%p\n", p[2]);
    printf("%p\n", p[3]);

    if (!(p[0] == p[1] && p[0] == p[2] && p[0] == p[3] && p[0] == p[4]))
        printf("FAILED h %d %d %d %d\n", p[0] == p[1], p[0] == p[2], p[0] == p[3], p[0] == p[4]);
    printf("%d\n", p[0] == NULL);

    p[0] = memrchr(buf+1, 'W', 27-1);
    p[1] = memrchr8(buf+1, 'W', 27-1);
    p[2] = memrchr8(buf+1, 'W', 27-1);
    p[3] = memrchr8(buf+1, 'W', 27-1);
    p[4] = memrchr8(buf+1, 'W', 27-1);

    if (!(p[0] == p[1] && p[0] == p[2] && p[0] == p[3] && p[0] == p[4]))
        printf("FAILED W %d %d %d %d\n", p[0] == p[1], p[0] == p[2], p[0] == p[3], p[0] == p[4]);
    printf("%s\n", p[0]);

    p[0] = memrchr(buf, 'k', 27);
    p[1] = memrchr8(buf, 'k', 27);
    p[2] = memrchr8(buf, 'k', 27);
    p[3] = memrchr8(buf, 'k', 27);
    p[4] = memrchr8(buf, 'k', 27);

    if (!(p[0] == p[1] && p[0] == p[2] && p[0] == p[3] && p[0] == p[4]))
        printf("FAILED k\n");
    printf("%d\n", p[0] == NULL);


    return(0);
}

