/* [ strbuf.h ] - String Buffer
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

#ifndef _STRBUF_H_
#define _STRBUF_H_

#include <stddef.h>

typedef struct _string {
    char * blob;
    size_t blksz;
    size_t size;
} strbuf_t;

strbuf_t *  strbuf_alloc        (strbuf_t *string);
void        strbuf_free         (strbuf_t *string);

int         strbuf_mmsqueeze    (strbuf_t *string);
int         strbuf_mmreserve    (strbuf_t *string,
                                 size_t n);

int         strbuf_clear        (strbuf_t *string);

int         strbuf_set          (strbuf_t *string,
                                 const char *blob,
                                 size_t n);

int         strbuf_append       (strbuf_t *string,
                                 const char *blob,
                                 size_t n);
int         strbuf_prepend      (strbuf_t *string,
                                 const char *blob,
                                 size_t n);
int         strbuf_insert       (strbuf_t *string,
                                 size_t index,
                                 const char *blob,
                                 size_t n);

int         strbuf_truncate     (strbuf_t *string,
                                 size_t n);
int         strbuf_remove       (strbuf_t *string,
                                 size_t index,
                                 size_t n);

int         strbuf_replace      (strbuf_t *string,
                                 size_t index,
                                 size_t size,
                                 const char *blob,
                                 size_t n);

int         strbuf_ltrim        (strbuf_t *string);
int         strbuf_rtrim        (strbuf_t *string);
int         strbuf_trim         (strbuf_t *string);

int         strbuf_equal        (strbuf_t *string,
                                 const char *blob,
                                 size_t n);
int         strbuf_compare      (strbuf_t *string,
                                 const char *blob,
                                 size_t n);


#endif /* !_STRBUF_H_ */

