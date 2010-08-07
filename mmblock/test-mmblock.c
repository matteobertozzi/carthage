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
#include "mmblock.h"

int main (int argc, char **argv) {
    mmblock_t bk0, bk1, bk2;
    mmblock_t block;
    char buf[16];

    if (mmblock_alloc(&block, NULL, 16) == NULL)
        return(1);

    if (mmblock_write(&block, "Hello", 0, 5) <= 0)
        return(1);
    mmblock_copy(&bk0, &block);

    if (mmblock_write(&block, "World", 8, 6) <= 0)
        return(1);
    mmblock_copy(&bk1, &block);

    if (mmblock_write(&block, " 2 ", 5, 3) <= 0)
        return(1);
    mmblock_copy(&bk2, &block);

    if (mmblock_write(&bk0, " bk1", 5, 4) <= 0)
        return(1);

    if (mmblock_write(&bk1, "bk2", 5, 3) <= 0)
        return(1);

    mmblock_read(&block, buf, 0, 16);
    printf("block: %p %s\n", block.d, buf);

    mmblock_read(&bk0, buf, 0, 16);
    printf("blk0:  %p %s\n", bk0.d, buf);

    mmblock_read(&bk1, buf, 0, 16);
    printf("blk1:  %p %s\n", bk1.d, buf);

    mmblock_read(&bk2, buf, 0, 16);
    printf("blk2:  %p %s\n", bk2.d, buf);

    mmblock_free(&block);
    mmblock_free(&bk0);
    mmblock_free(&bk1);
    mmblock_free(&bk2);

    return(0);
}

