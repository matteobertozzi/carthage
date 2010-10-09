/* [ uintx.h ] - Unsigned Int of specified bit length.
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

#ifndef _UINTX_H_
#define _UINTX_H_

#include <stdlib.h>
#include <stdint.h>

#define UINTX_DEFINE(bits)          \
    typedef uint8_t uintx ## bits ## _t[(((bits) + 7) & (-8)) >> 3];

void  uintx_init          (uint8_t *x, size_t n);
void  uintx_from_u64      (uint8_t *x, size_t n, uint64_t value);
int   uintx_is_zero       (uint8_t *x, size_t n);

int   uintx_inc           (uint8_t *x, size_t n);
int   uintx_dec           (uint8_t *x, size_t n);

void  uintx_add           (uint8_t *a, size_t an, const uint8_t *b, size_t bn);
void  uintx_add64         (uint8_t *x, size_t n, uint64_t value);

void  uintx_and           (uint8_t *a, size_t an, const uint8_t *b, size_t bn);
void  uintx_and64         (uint8_t *x, size_t n, uint64_t value);

void  uintx_or            (uint8_t *a, size_t an, const uint8_t *b, size_t bn);
void  uintx_or64          (uint8_t *x, size_t n, uint64_t value);

void  uintx_xor           (uint8_t *a, size_t an, const uint8_t *b, size_t bn);
void  uintx_xor64         (uint8_t *x, size_t n, uint64_t value);

int   uintx_compare       (const uint8_t *a, size_t an,
                           const uint8_t *b, size_t bn);
int   uintx_compare64     (const uint8_t *x, size_t n, uint64_t value);

void  uintx_print         (const uint8_t *x, size_t n);

#endif /* !_UINTX_H_ */

