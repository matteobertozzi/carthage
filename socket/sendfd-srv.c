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

#include <sys/ioctl.h>
#include <stropts.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "sockpoll.h"
#include "socket.h"

static int __accept (int socket,
                     void *user_data)
{
    struct sockaddr_storage address;
    char ip[INET6_ADDRSTRLEN];
    int client;
    int xfd;

    if ((client = socket_tcp_accept(socket, &address)) < 0)
        return(-1);

    socket_str_address(ip, INET6_ADDRSTRLEN, (const struct sockaddr *)&address);
    printf(" - Listener is ready to accept %d %s.\n", client, ip);

    /* Send the file descriptor to the other server */
    xfd = socket_unix_sendfd(*((int *)user_data), client);
    printf("   - Sended: %d\n", xfd);

    close(client);
    return(-1);
}

int main (int argc, char **argv) {
    struct sockaddr_storage addr;
    char ip[INET6_ADDRSTRLEN];
    int usock;
    int sock;

    if ((sock = socket_tcp_bind(NULL, "8080", &addr)) < 0)
        return(1);

    if ((usock = socket_unix_connect("test.sock")) < 0) {
        close(usock);
        return(2);
    }

    socket_str_address(ip, INET6_ADDRSTRLEN, (const struct sockaddr *)&addr);
    printf("Server is Listening on %s\n", ip);

#if defined(HAS_SOCKPOLL_EPOLL)
    printf("Using epoll...\n");
    sockpoll_epoll(sock, __accept, NULL, NULL, &usock);
#elif defined(HAS_SOCKPOLL_KQUEUE)
    printf("Using kqueue...\n");
    sockpoll_kqueue(sock, __accept, NULL, NULL, &usock);
#else
    printf("Using select...\n");
    sockpoll_select(sock, __accept, NULL, NULL, &usock);
#endif
    close(sock);

    return(0);
}

