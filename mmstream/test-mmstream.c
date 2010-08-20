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

#include "mmstream.h"

int main (int argc, char **argv) {
    char buffer[64];
    mmstream_t flow;
    int32_t n;

    mmstream_alloc(&flow, NULL);

    n = mmstream_write(&flow, "Test", 0U, 4U);
    printf("WRITE %d\n", n);

    n = mmstream_write(&flow, "Bitmap", 6U, 6U);
    printf("WRITE %d\n", n);

    memset(buffer, 0, sizeof(buffer));
    n = mmstream_read(&flow, buffer, 0U, 14U);
    printf("READ %d '%s'\n", n, buffer);

    n = mmstream_write(&flow, "Hello World", 0U, 11U);
    printf("WRITE %d\n", n);

    memset(buffer, 0, sizeof(buffer));
    mmstream_read(&flow, buffer, 6, 5);
    printf("READ %d '%s'\n", n, buffer);

    n = mmstream_write(&flow, "Double World", 6U, 13U);
    printf("WRITE %d\n", n);

    memset(buffer, 0, sizeof(buffer));
    n = mmstream_read(&flow, buffer, 6, 5);
    printf("READ %d '%s'\n", n, buffer);

    n = mmstream_write(&flow, ": This is just a test", 18U, 21U);
    printf("WRITE %d\n", n);

    memset(buffer, 0, sizeof(buffer));
    n = mmstream_read(&flow, buffer, 0U, 40U);
    printf("READ %d '%s'\n", n, buffer);
    printf("EXT: %llu\n", flow.extent);

    mmstream_truncate(&flow, 10U);

    memset(buffer, 0, sizeof(buffer));
    n = mmstream_read(&flow, buffer, 0U, 40U);
    printf("READ %d '%s'\n", n, buffer);
    printf("EXT: %llu\n", flow.extent);

    mmstream_free(&flow);

    return(0);
}

