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

static int __accept (void *user_data, int socket) {
    struct sockaddr_storage address;
    char ip[INET6_ADDRSTRLEN];
    int client;

    if ((client = socket_tcp_accept(socket, &address)) < 0)
        return(-1);

    socket_str_address(ip, INET6_ADDRSTRLEN, (const struct sockaddr *)&address);
    printf(" - Listener is ready to accept %d %s.\n", client, ip);

    return(client);
}

static int __read (void *user_data, int sock) {
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
    struct sockaddr_storage addr;
    char ip[INET6_ADDRSTRLEN];
    int sock;

    if ((sock = socket_tcp_bind(NULL, "8080", &addr)) < 0)
        return(1);

    socket_str_address(ip, INET6_ADDRSTRLEN, (const struct sockaddr *)&addr);
    printf("Server is Listening on %s\n", ip);

    sockpoll_exec(sock, __read, NULL, __accept, 0, NULL, NULL);

    close(sock);

    return(0);
}

