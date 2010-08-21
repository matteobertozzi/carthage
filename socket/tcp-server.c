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

#include "socket.h"

int main (int argc, char **argv) {
    struct sockaddr_storage addr;
    char ip[INET6_ADDRSTRLEN];
    char buffer[64];
    int client;
    int sock;
    int i;

    if ((sock = socket_tcp_bind(NULL, "8080", &addr)) < 0)
        return(1);

    socket_address(ip, INET6_ADDRSTRLEN, (const struct sockaddr *)&addr);
    printf("Server is Listening on %s\n", ip);

    for (i = 0; i < 3; ++i) {
        if ((client = socket_tcp_accept(sock, &addr)) < 0)
            continue;

        socket_address(ip, INET6_ADDRSTRLEN, (const struct sockaddr *)&addr);
        printf(" - Client %s connected.\n", ip);

        if (send(client, "Hello Client\n", 13, 0) != 13)
            perror("send()");

        memset(buffer, 0, 64);
        if (recv(client, buffer, 64, 0) < 0)
            perror("recv()");
        printf("Recv: %s\n", buffer);

        close(client);
    }

    close(sock);

    return(0);
}

