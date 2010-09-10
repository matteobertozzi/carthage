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

#include "strupper.h"
#include "strlower.h"
#include "strtrim.h"
#include "strstr.h"
#include "strcmp.h"
#include "strlen.h"

static void __test_strstr (void) {
    char haystack[] = "Woo Hello World, Woo this is Haystack";
    size_t haystack_len;
    ssize_t x;
    char *p;

    haystack_len = sizeof(haystack);

    p = strstr(haystack, "World");
    x = strpos(haystack, "World");
    printf("%2lu %2ld %s\n", p - haystack, x, p);    

    p = strstr(haystack, "Woo");
    x = strpos(haystack, "Woo");
    printf("%2lu %2ld %s\n", p - haystack, x, p);

    p = strrstr(haystack, "Woo");
    x = strrpos(haystack, "Woo");
    printf("%2lu %2ld %s\n", p - haystack, x, p);

    p = strstr(haystack, "thesht");
    x = strpos(haystack, "thesht");
    printf("p is NULL %d: %2ld\n", p == NULL, x);

    p = strrstr(haystack, "thesht");
    x = strrpos(haystack, "thesht");
    printf("p is NULL %d - %2ld\n", p == NULL, x);
}

static void __test_strcase (void) {
    char s1[] = "HeLlo WoRld! -Th1s 73sT#";
    char s2[] = "HeLlo WoRld! -Th1s 73sT#";
    
    strupper(s1);
    strlower(s2);

    printf("S1 %s\n", s1);
    printf("S2 %s\n", s2);
    printf("CASE CMP: %d\n", strcasecmp(s1, s2));
}

int main (int argc, char **argv) {
    __test_strstr();
    __test_strcase();
    return(0);
}

