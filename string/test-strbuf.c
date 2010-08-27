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

#include "strbuf.h"

int main (int argc, char **argv) {
    strbuf_t str;

    strbuf_alloc(&str);
    strbuf_append(&str, "Hello", 5);
    printf("%2lu (%2lu): %s\n", str.blksz, str.size, str.blob);
    strbuf_append(&str, "World", 5);
    printf("%2lu (%2lu): %s\n", str.blksz, str.size, str.blob);
    strbuf_insert(&str, 5, " ", 1);
    printf("%2lu (%2lu): %s\n", str.blksz, str.size, str.blob);
    strbuf_prepend(&str, "This is: ", 9);
    printf("%2lu (%2lu): %s\n", str.blksz, str.size, str.blob);

    strbuf_replace(&str, 15, 5, "dlroW", 5);
    printf("%2lu (%2lu): %s\n", str.blksz, str.size, str.blob);

    strbuf_replace(&str, 10, 4, str.blob + 10, 5);
    printf("%2lu (%2lu): %s\n", str.blksz, str.size, str.blob);

    strbuf_replace(&str, 0, 4, "What ", 5);
    printf("%2lu (%2lu): %s\n", str.blksz, str.size, str.blob);

    printf("mmsqueeze: %d\n", strbuf_mmsqueeze(&str));
    printf("%2lu (%2lu): %s\n", str.blksz, str.size, str.blob);

    strbuf_set(&str, "   Test String    ", 18);
    printf("%2lu (%2lu): '%s'\n", str.blksz, str.size, str.blob);

    strbuf_rtrim(&str);
    printf("%2lu (%2lu): '%s'\n", str.blksz, str.size, str.blob);
    strbuf_ltrim(&str);
    printf("%2lu (%2lu): '%s'\n", str.blksz, str.size, str.blob);

    strbuf_set(&str, "Hello", 5);
    printf("%d %d\n", strbuf_equal(&str, "Hello", 5), strbuf_equal(&str, "Belo", 4));
    printf("%d %d %d\n", strbuf_compare(&str, "Hello", 5), strbuf_compare(&str, "Belo", 4), strbuf_compare(&str, "Zelo", 4));

    strbuf_prepend(&str, str.blob, 5);
    printf("%2lu (%2lu): '%s'\n", str.blksz, str.size, str.blob);

    strbuf_insert(&str, 16, "Woo", 3);
    printf("%2lu (%2lu): '%s'\n", str.blksz, str.size, str.blob);

    strbuf_free(&str);

    return(0);
}

