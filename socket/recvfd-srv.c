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

static int __accept (void *user_data, int sock) {
    int client;

    if ((client = socket_unix_accept(sock)) < 0)
        return(-1);

    printf(" - Listener is ready to accept %d.\n", client);
    return(client);
}

static int __read (void *user_data, int sock) {
    ssize_t n;
    int xfd;

    if ((xfd = socket_unix_recvfd(sock)) < 0)
        return(0);

    printf(" - %d is ready to read.\n", sock);
    printf(" - XFD: %d\n", xfd);

    n = send(xfd, "Hello from Unix Server\n", 23, 0);
    printf("Sended: %ld\n", n);

    close(xfd);
    return(0);
}


int main (int argc, char **argv) {
    int sock;

    if ((sock = socket_unix_bind("test.sock", 0)) < 0)
        return(1);

    printf("Server is Listening...\n");
    sockpoll_exec(sock, __read, NULL, __accept, 0, NULL, NULL);

    close(sock);

    return(0);
}

