/* [ socket.h ] - Socket
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
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/socket.h>
#include <netdb.h>

int     socket_tcp_connect      (const char *address,
                                 const char *port,
                                 struct sockaddr_storage *addr);
int     socket_tcp_bind         (const char *address,
                                 const char *port,
                                 struct sockaddr_storage *addr);
int     socket_tcp_accept       (int socket,
                                 struct sockaddr_storage *addr);


int     socket_udp_bind         (const char *address,
                                 const char *port,
                                 struct sockaddr_storage *addr);
int     socket_udp_connect      (const char *address,
                                 const char *port,
                                 struct sockaddr_storage *addr);
int     socket_udp_send         (int sock,
                                 const struct sockaddr_storage *addr,
                                 const void *buffer,
                                 int n,
                                 int flags);
int     socket_udp_recv         (int sock,
                                 struct sockaddr_storage *addr,
                                 void *buffer,
                                 int n,
                                 int flags);

#ifdef HAS_UNIX_SOCKET

#include <sys/un.h>

int     socket_unix_connect     (const char *filepath);
int     socket_unix_bind        (const char *filepath, int dgram);
int     socket_unix_accept      (int socket);

int     socket_unix_send        (int sock,
                                 const struct sockaddr_un *addr,
                                 const void *buffer,
                                 int n,
                                 int flags);
int     socket_unix_recv        (int sock,
                                 struct sockaddr_un *addr,
                                 void *buffer,
                                 int n,
                                 int flags);
#endif /* HAS_UNIX_SOCKET */

int     socket_address          (int sock,
                                 struct sockaddr_storage *addr);

char *  socket_str_address      (char *buffer,
                                 size_t n,
                                 const struct sockaddr *address);
char *  socket_str_address_info (char *buffer,
                                 size_t n,
                                 const struct addrinfo *address);

int     socket_address_is_ipv6  (const struct sockaddr *address);

#endif /* !_SOCKET_H_ */

