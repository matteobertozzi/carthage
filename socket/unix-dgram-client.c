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

#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "socket.h"

#define __UNIX_SOCK_PATH     "client-dgram.sock"
#define __UNIX_SOCK_SRVPATH  "server-dgram.sock"

#define __QUIT_SOCK(sock)               \
    do {                                \
        close(sock);                    \
        unlink(__UNIX_SOCK_PATH);       \
    } while (0)

int main (int argc, char **argv) {
    struct sockaddr_un addr;
    socklen_t addrlen;
    char buffer[64];
    int sock;
    int n;

    if ((sock = socket_unix_bind(__UNIX_SOCK_PATH, 1)) < 0)
        return(1);

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, __UNIX_SOCK_SRVPATH);
    if ((n = socket_unix_send(sock, &addr, "Hello Server\n", 13, 0)) < 0) {
        perror("send()");
        __QUIT_SOCK(sock);
        return(2);
    }
    printf("Send %d\n", n);

    memset(buffer, 0, sizeof(buffer));
    addrlen = sizeof(struct sockaddr_un);
    if ((n = socket_unix_recv(sock, &addr, buffer, 64, 0)) < 0) {
        perror("recvfrom()");
        __QUIT_SOCK(sock);
        return(3);
    }
    printf("Recv %s: %s\n", addr.sun_path, buffer);

    __QUIT_SOCK(sock);

    return(0);
}

