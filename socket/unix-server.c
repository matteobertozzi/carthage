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
#include <unistd.h>
#include <stdio.h>

#include "sockpoll.h"
#include "socket.h"

static int __accept (int sock, void *user_data) {
    int client;

    if ((client = socket_unix_accept(sock)) < 0)
        return(-1);

    printf(" - Listener is ready to accept %d.\n", client);
    return(client);
}

static int __read (int sock, void *user_data)
{
    char buffer[1024];
    ssize_t n;

    memset(buffer, 0, 1024);
    if ((n = recv(sock, buffer, 1024, 0)) < 1) {
        printf(" - %d is ready to leave.\n", sock);
        return(-1);
    }

    printf(" - %d is ready to read %d.\n", sock, n);
    printf("   %s\n", buffer);

    send(sock, "OK, Received!\n", 14, 0);

    return(0);
}


int main (int argc, char **argv) {
    int sock;

    if ((sock = socket_unix_bind("test.sock")) < 0)
        return(1);

    printf("Server is Listening...\n");

#if defined(HAS_SOCKPOLL_EPOLL)
    printf("Using epoll...\n");
    sockpoll_epoll(sock, __accept, __read, NULL, NULL);
#elif defined(HAS_SOCKPOLL_KQUEUE)
    printf("Using kqueue...\n");
    sockpoll_kqueue(sock, __accept, __read, NULL, NULL);
#else
    printf("Using select...\n");
    sockpoll_select(sock, __accept, __read, NULL, NULL);
#endif
    close(sock);

    return(0);
}

