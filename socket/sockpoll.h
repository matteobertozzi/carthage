/* [ sockpoll.h ] - Socket (Event Loop) Pool
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
#ifndef _SOCKPOLL_H_
#define _SOCKPOLL_H_

#include "iopoll.h"

typedef int (*socket_accept_t) (void *user_data, int sock);

typedef struct _sockpoll {
    void *          user_data;
    socket_accept_t accept_f;
    iowrite_t       write_f;
    ioread_t        read_f;
    iopoll_t        iopoll;
    int             sock;
} sockpoll_t;

sockpoll_t *sockpoll_alloc      (sockpoll_t *sockpoll,
                                 int server_sock,
                                 ioread_t read_f,
                                 iowrite_t write_f,
                                 socket_accept_t accept_f,
                                 iopoll_backend_type_t type,
                                 int timeout,
                                 int *is_looping,
                                 void *user_data);
void        sockpoll_free       (sockpoll_t *sockpoll);
int         sockpoll_add        (sockpoll_t *sockpoll, int sock);
int         sockpoll_remove     (sockpoll_t *sockpoll, int sock);
int         sockpoll_loop       (sockpoll_t *sockpoll);

int         sockpoll_exec       (int socket,
                                 ioread_t read_f,
                                 iowrite_t write_f,
                                 socket_accept_t accept_f,
                                 iopoll_backend_type_t type,
                                 int *is_looping,
                                 void *user_data);

#endif /* !_SOCKPOLL_H_ */

