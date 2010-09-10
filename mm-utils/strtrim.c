/* [ strtrim.c ] - String Trim
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

#include "strtrim.h"
#include "strlen.h"
#include "memcpy.h"

size_t strnltrim (char *str, size_t n) {
    size_t start = 0U;

    while (isspace(str[start]) && start < n)
        start++;

    if (start > 0U) {
        memmove(str, str + start, n - start);
        str[n - start] = '\0';
    }

    return(n);
}

size_t strnrtrim (char *str, size_t n) {
    while (n > 0U && isspace(str[n - 1]))
        n--;

    str[n] = '\0';
    return(n);
}

size_t strntrim (char *str, size_t n) {
    n = strnrtrim(str, n);
    n = strnltrim(str, n);
    return(n);
}

char *strltrim (char *str) {
    strnltrim(str, strlen(str));
    return(str);
}

char *strrtrim (char *str) {
    strnrtrim(str, strlen(str));
    return(str);
}

char *strtrim (char *str) {
    strntrim(str, strlen(str));
    return(str);
}

