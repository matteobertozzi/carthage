/* [ strcmp.c ] - compare two strings
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
#include <ctype.h>

#include "memcmp.h"
#include "strlen.h"

#define __tolower(c)          ((!isalpha(c) || islower(c)) ? (c) : ((c) - 'A' + 'a'))

int strncmp (const char *s1, const char *s2, size_t n) {
    size_t l1, l2;

    if ((l1 = strlen(s1)) < n) n = l1;
    if ((l2 = strlen(s2)) < n) n = l2;

    return(memcmp(s1, s2, n));
}

int strcmp (const char *s1, const char *s2) {
    size_t l1, l2;

    l1 = strlen(s1);
    l2 = strlen(s2);

    return(memcmp(s1, s2, l1 < l2 ? l1 : l2));
}

int strncasecmp (const char *s1, const char *s2, size_t n) {
    unsigned char c1, c2;

    while (n--) {
        c1 = *s1++;
        c2 = *s2++;
        if ((c1 = __tolower(c1)) != (c2 = __tolower(c2)))
            return(c1 - c2);

        if (!c1)
            break;
    };

    return(0);
}

int strcasecmp (const char *s1, const char *s2) {
    register unsigned char c1, c2;

    do {
        c1 = *s1++;
        c2 = *s2++;
        if ((c1 = __tolower(c1)) != (c2 = __tolower(c2)))
            return(c1 - c2);

    } while (c1);

    return(0);
}

