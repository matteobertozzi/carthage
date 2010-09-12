/* [ sockpoll.c ] - Socket (Event Loop) Pool
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

#include <unistd.h>
#include "sockpoll.h"

static int __sockpoll_read (void *user_data, int sock) {
    sockpoll_t *sockpoll = (sockpoll_t *)user_data;

    if (sock == sockpoll->sock) {
        int client;

        if ((client = sockpoll->accept_f(sockpoll->user_data, sock)) < 0) {
            close(client); /* ?? */
            return(-1);
        }

        iopoll_add(&(sockpoll->iopoll), client);
    } else {
        ioread_t read_f = sockpoll->read_f;

        if (!read_f || read_f(sockpoll->user_data, sock) < 0) {
            iopoll_remove(&(sockpoll->iopoll), sock);
            close(sock);
            return(-2);
        }
    }

    return(0);
}

static int __sockpoll_write (void *user_data, int sock) {
    sockpoll_t *sockpoll = (sockpoll_t *)user_data;
    return(sockpoll->write_f(user_data, sock));
}

sockpoll_t *sockpoll_alloc (sockpoll_t *sockpoll,
                            int server_sock,
                            ioread_t read_f,
                            iowrite_t write_f,
                            socket_accept_t accept_f,
                            iopoll_backend_type_t type,
                            int timeout,
                            int *is_looping,
                            void *user_data)
{
    iowrite_t sp_write_f = NULL;

    sockpoll->read_f = read_f;
    if ((sockpoll->write_f = write_f) != NULL)
        sp_write_f = __sockpoll_write;
    sockpoll->accept_f = accept_f;

    sockpoll->sock = server_sock;
    sockpoll->user_data = user_data;

    iopoll_alloc(&(sockpoll->iopoll),
                        __sockpoll_read,
                        sp_write_f,
                        type,
                        timeout,
                        is_looping,
                        sockpoll);

    iopoll_add(&(sockpoll->iopoll), server_sock);

    return(sockpoll);
}

void sockpoll_free (sockpoll_t *sockpoll) {
    iopoll_free(&(sockpoll->iopoll));
}

int sockpoll_add (sockpoll_t *sockpoll, int sock) {
    return(iopoll_add(&(sockpoll->iopoll), sock));
}

int sockpoll_remove (sockpoll_t *sockpoll, int sock) {
    return(iopoll_remove(&(sockpoll->iopoll), sock));
}

int sockpoll_loop (sockpoll_t *sockpoll) {
    return(iopoll_loop(&(sockpoll->iopoll)));
}

int sockpoll_exec (int socket,
                   ioread_t read_f,
                   iowrite_t write_f,
                   socket_accept_t accept_f,
                   iopoll_backend_type_t type,
                   int *is_looping,
                   void *user_data)
{
    sockpoll_t poll;
    int res;

    if (sockpoll_alloc(&poll, socket,
                       read_f, write_f, accept_f,
                       type, 0, is_looping, user_data) == NULL)
    {
        return(-1);
    }

    res = sockpoll_loop(&poll);

    sockpoll_free(&poll);

    return(res);
}

