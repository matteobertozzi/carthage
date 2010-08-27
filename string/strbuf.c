/* [ strbuf.c ] - String Buffer
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
#include <string.h>
#include <ctype.h>

#include "strbuf.h"

#define __align_up(x, align)            (((x) + ((align) - 1)) & (-(align)))

#define __mmrealloc(str, ptr, size)     realloc((ptr), (size))
#define __mmfree(str, ptr)              free(ptr)

static int __strbuf_realloc (strbuf_t *string, size_t size) {
    char *blob;

    /* Adjust string size */
    if (size < 32)
        size = __align_up(size, 8);
    else if (size < 128)
        size = __align_up(size, 16);
    else if (size < 512)
        size = __align_up(size, 64);
    else
        size = __align_up(size, 1024);

    /* Realloc string buffer blob */
    if ((blob = __mmrealloc(string, string->blob, size)) == NULL)
        return(-1);

    string->blksz = size;
    string->blob = blob;

    return(0);
}

strbuf_t *strbuf_alloc (strbuf_t *string) {
    string->blob = NULL;
    string->blksz = 0U;
    string->size = 0U;
    return(string);
}

void strbuf_free (strbuf_t *string) {
    if (string->blob != NULL) {
        __mmfree(string, string->blob);
        string->blob = NULL;
    }

    string->blksz = 0U;
    string->size = 0U;
}

int strbuf_mmsqueeze (strbuf_t *string) {
    char *blob;

    if ((blob = __mmrealloc(string, string->blob, string->size)) == NULL)
        return(-1);

    string->blksz = string->size;
    string->blob = blob;
    return(0);
}

int strbuf_mmreserve (strbuf_t *string,
                      size_t n)
{
    if (n > string->blksz) {
        if (__strbuf_realloc(string, n))
            return(-1);
    }
    return(0);
}

int strbuf_clear (strbuf_t *string) {
    string->blob[0] = '\0';
    string->size = 0;
    return(0);
}

int strbuf_set (strbuf_t *string,
                const char *blob,
                size_t n)
{
    /* Realloc String Buffer if necessary */
    if (n > string->blksz) {
        if (__strbuf_realloc(string, n))
            return(-1);
    }

    memmove(string->blob, blob, n);
    string->blob[n] = '\0';
    string->size = n;

    return(0);
}

int strbuf_append (strbuf_t *string,
                   const char *blob,
                   size_t n)
{
    size_t size;

    /* Realloc String Buffer if necessary */
    if ((size = (string->size + n)) > string->blksz) {
        if (__strbuf_realloc(string, size))
            return(-1);
    }

    memcpy(string->blob + string->size, blob, n);
    string->blob[size] = '\0';
    string->size = size;

    return(0);
}

int strbuf_prepend (strbuf_t *string,
                    const char *blob,
                    size_t n)
{
    size_t size;

    /* Realloc String Buffer if necessary */
    if ((size = (string->size + n)) > string->blksz) {
        if (__strbuf_realloc(string, size))
            return(-1);
    }

    if (blob >= string->blob && blob < (string->blob + string->blksz)) {
        char *dblob;

        if ((dblob = strndup(blob, n)) == NULL)
            return(-1);

        memmove(string->blob + n, string->blob, string->size);
        memcpy(string->blob, dblob, n);

        free(dblob);
    } else {
        memmove(string->blob + n, string->blob, string->size);
        memcpy(string->blob, blob, n);
    }

    string->blob[size] = '\0';
    string->size = size;

    return(0);
}

int strbuf_insert (strbuf_t *string,
                   size_t index,
                   const char *blob,
                   size_t n)
{
    size_t size;
    char *p;

    size = string->size + n;
    if (index > string->size)
        size += (index - string->size);

    /* Realloc String Buffer if necessary */
    if (size > string->blksz) {
        if (__strbuf_realloc(string, size))
            return(-1);
    }

    p = (string->blob + index);
    if (index > string->size) {
        memset(string->blob + string->size, ' ', index - string->size);
        memcpy(p, blob, n);
    } else if (blob >= string->blob && blob < (string->blob + string->blksz)) {
        char *dblob;

        if ((dblob = strndup(blob, n)) == NULL)
            return(-1);

        memmove(p + n, p, string->size - index);
        memcpy(p, dblob, n);

        free(dblob);
    } else {
        memmove(p + n, p, string->size - index);
        memcpy(p, blob, n);
    }

    string->blob[size] = '\0';
    string->size = size;

    return(0);
}

int strbuf_truncate (strbuf_t *string,
                     size_t n)
{
    if (n >= string->size)
        return(-1);

    string->blob[n] = '\0';
    string->size = n;

    return(0);
}

int strbuf_remove (strbuf_t *string,
                   size_t index,
                   size_t n)
{
    if (index >= string->size || n == 0)
        return(-1);

    if ((index + n) >= string->size) {
        string->size = index;
    } else {
        char *p;

        p = string->blob + index;
        memmove(p, p + n, string->size - index - n);
        string->size -= n;
    }

    string->blob[string->size] = '\0';

    return(0);
}

int strbuf_replace (strbuf_t *string,
                    size_t index,
                    size_t size,
                    const char *blob,
                    size_t n)
{
    if (size == n && (index + n) <= string->size) {
        memmove(string->blob + index, blob, n);
    } else {
        strbuf_remove(string, index, n);
        if (strbuf_insert(string, index, blob, n))
            return(-3);
    }

    return(0);
}

int strbuf_ltrim (strbuf_t *string) {
    size_t start = 0U;

    while (isspace(string->blob[start]) && start < string->size)
        start++;

    if (start > 0U) {
        memmove(string->blob, string->blob + start, string->size - start);

        string->size -= start;
        string->blob[string->size] = '\0';
    }

    return(0);
}

int strbuf_rtrim (strbuf_t *string) {
    size_t end = string->size;

    while (end > 0U && isspace(string->blob[end - 1]))
        end--;

    string->size = end;
    string->blob[end] = '\0';

    return(0);
}

int strbuf_trim (strbuf_t *string) {
    strbuf_rtrim(string);
    strbuf_ltrim(string);
    return(0);
}

int strbuf_equal (strbuf_t *string,
                  const char *blob,
                  size_t n)
{
    if (string->size != n)
        return(0);
    return(!memcmp(string->blob, blob, n));
}

int strbuf_compare (strbuf_t *string,
                    const char *blob,
                    size_t n)
{
    if (string->size < n)
        n = string->size;
    return(memcmp(string->blob, blob, n));
}

