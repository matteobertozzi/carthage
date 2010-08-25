/* [ socket.c ] - Socket
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

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>

#define __LISTEN_BACKLOG        (16)

/* ===========================================================================
 *  Socket Private Helpers
 */
static int __socket (struct addrinfo *info,
                     int set_reuse_addr,
                     int set_reuse_port)
{
    int s;

    if ((s = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) < 0)
        return(s);

    if (set_reuse_addr) {
        int yep = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yep, sizeof(int));
    }

    if (set_reuse_port) {
#if defined(SO_REUSEPORT)
        int yep = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &yep, sizeof(int));
#endif /* SO_REUSEPORT */
    }

    return(s);
}


static int __socket_bind (const char *address,
                          const char *port,
                          struct addrinfo *hints,
                          struct sockaddr_storage *addr)
{
    struct addrinfo *info;
    struct addrinfo *p;
    int status;
    int sock;

    if ((status = getaddrinfo(address, port, hints, &info)) != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(status));
        return(-1);
    }

    for (p = info; p != NULL; p = p->ai_next) {
        if ((sock = __socket(p, 1, 1)) < 0) {
            perror("socket()");
            continue;
        }

        if (bind(sock, info->ai_addr, info->ai_addrlen) < 0) {
            perror("bind()");
            close(sock);
            sock = -1;
            continue;
        }

        if (addr != NULL)
            memcpy(addr, p->ai_addr, p->ai_addrlen);

        break;
    }

    freeaddrinfo(info);
    return(sock);
}

/* ===========================================================================
 *  TCP Socket Related
 */
int socket_tcp_connect (const char *address,
                        const char *port,
                        struct sockaddr_storage *addr)
{
    struct addrinfo hints;
    struct addrinfo *info;
    struct addrinfo *p;
    int status;
    int sock;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(address, port, &hints, &info)) != 0) {
        fprintf(stderr, "getaddrinfo(): %s", gai_strerror(status));
        return(-1);
    }

    for (p = info; p != NULL; p = p->ai_next) {
        if ((sock = __socket(p, 0, 0)) < 0) {
            perror("socket()");
            continue;
        }

        if (connect(sock, p->ai_addr, p->ai_addrlen) < 0) {
            perror("connect()");
            close(sock);
            sock = -1;
            continue;
        }

        if (addr != NULL)
            memcpy(addr, p->ai_addr, p->ai_addrlen);

        break;
    }

    freeaddrinfo(info);
    return(sock);
}

int socket_tcp_bind (const char *address,
                     const char *port,
                     struct sockaddr_storage *addr)
{
    struct addrinfo hints;
    int sock;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if ((sock = __socket_bind(address, port, &hints, addr)) < 0)
        return(sock);

    if (listen(sock, __LISTEN_BACKLOG) < 0) {
        perror("listen()");
        close(sock);
        return(-2);
    }

    return(sock);
}

int socket_tcp_accept (int socket,
                       struct sockaddr_storage *addr)
{
    struct sockaddr_storage address;
    socklen_t addr_size;
    int sd;

    addr_size = sizeof(struct sockaddr_storage);
    if ((sd = accept(socket, (struct sockaddr *)&address, &addr_size)) < 0) {
        perror("accept()");
        return(-1);
    }

    if (addr != NULL)
        memcpy(addr, &address, sizeof(struct sockaddr_storage));

    return(sd);
}

/* ===========================================================================
 *  UDP Socket Related
 */
int socket_udp_bind (const char *address,
                     const char *port,
                     struct sockaddr_storage *addr)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    return(__socket_bind(address, port, &hints, addr));
}

int socket_udp_connect (const char *address,
                        const char *port,
                        struct sockaddr_storage *addr)
{
    struct addrinfo hints;
    struct addrinfo *info;
    struct addrinfo *p;
    int status;
    int sock;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    if ((status = getaddrinfo(address, port, &hints, &info)) != 0) {
        fprintf(stderr, "getaddrinfo(): %s", gai_strerror(status));
        return(-1);
    }

    for (p = info; p != NULL; p = p->ai_next) {
        if ((sock = __socket(p, 0, 0)) < 0) {
            perror("socket()");
            continue;
        }

        if (addr != NULL)
            memcpy(addr, p->ai_addr, p->ai_addrlen);

        break;
    }

    freeaddrinfo(info);
    return(sock);
}

int socket_udp_send (int sock,
                     const struct sockaddr_storage *addr,
                     const void *buffer,
                     int n,
                     int flags)
{
    return(sendto(sock, buffer, n, flags,
                  (const struct sockaddr *)addr,
                  sizeof(struct sockaddr_storage)));
}

int socket_udp_recv (int sock,
                     struct sockaddr_storage *addr,
                     void *buffer,
                     int n,
                     int flags)
{
    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_storage);
    return(recvfrom(sock, buffer, n, flags,
                    (struct sockaddr *)addr,
                    &addr_size));
}

/* ===========================================================================
 *  UNIX Socket Related
 */
#ifdef HAS_UNIX_SOCKET

#include <sys/un.h>

int socket_unix_connect (const char *filepath) {
    struct sockaddr_un addr;
    socklen_t addrlen;
    int sock;

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return(-1);
    }

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, filepath);

    addrlen = (strlen(addr.sun_path) + (sizeof(addr) - sizeof(addr.sun_path)));
    if (connect(sock, (struct sockaddr *)&addr, addrlen) < 0) {
        perror("connect()");
        close(sock);
        return(-2);
    }

    return(sock);
}

int socket_unix_bind (const char *filepath, int dgram) {
    struct sockaddr_un addr;
    socklen_t addrlen;
    int sock;

    if ((sock = socket(AF_UNIX, dgram ? SOCK_DGRAM : SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return(-2);
    }

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, filepath);

    addrlen = (strlen(addr.sun_path) + (sizeof(addr) - sizeof(addr.sun_path)));
    if (bind(sock, (struct sockaddr *)&addr, addrlen) < 0) {
        perror("bind()");
        close(sock);
        return(-3);
    }

    if (!dgram) {
        if (listen(sock, __LISTEN_BACKLOG) < 0) {
            perror("listen()");
            unlink(addr.sun_path);
            close(sock);
            return(-4);
        }
    }

    return(sock);
}

int socket_unix_accept (int socket) {
    struct sockaddr_un address;
    socklen_t addr_size;
    int sd;

    addr_size = sizeof(struct sockaddr_un);
    if ((sd = accept(socket, (struct sockaddr *)&address, &addr_size)) < 0) {
        perror("accept()");
        return(-1);
    }

    return(sd);
}

int socket_unix_sendfd (int sock, int fd) {
    unsigned char cmsgbuf[CMSG_LEN(sizeof(int))];
    struct cmsghdr *cmsg;
    unsigned int magic;
    struct msghdr msg;
    struct iovec iov;

    magic = 0xFD5;
    iov.iov_base = &magic;
    iov.iov_len = sizeof(unsigned int);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    cmsg = (struct cmsghdr *)cmsgbuf;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    msg.msg_control = cmsg;
    msg.msg_controllen = cmsg->cmsg_len;
    *(int *)CMSG_DATA(cmsg) = fd;

    if (sendmsg(sock, &msg, 0) != sizeof(unsigned int)) {
        perror("sendmsg()");
        return(-1);
    }

    return(0);
}

int socket_unix_recvfd (int sock) {
    unsigned char cmsgbuf[CMSG_LEN(sizeof(int))];
    struct cmsghdr *cmsg;
    struct msghdr msg;
    struct iovec iov;
    unsigned int magic;

    iov.iov_base = &magic;
    iov.iov_len = sizeof(unsigned int);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    cmsg = (struct cmsghdr *)cmsgbuf;
    msg.msg_control = cmsg;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    if (recvmsg(sock, &msg, 0) != sizeof(unsigned int)) {
        perror("recvmsg()");
        return(-1);
    }

    if (magic != 0xFD5)
        return(-2);

    return(*(int *)CMSG_DATA(cmsg));
}

int socket_unix_send (int sock,
                      const struct sockaddr_un *addr,
                      const void *buffer,
                      int n,
                      int flags)
{
    return(sendto(sock, buffer, n, flags,
                  (const struct sockaddr *)addr,
                  sizeof(struct sockaddr_un)));
}

int socket_unix_recv (int sock,
                      struct sockaddr_un *addr,
                      void *buffer,
                      int n,
                      int flags)
{
    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_un);
    return(recvfrom(sock, buffer, n, flags,
                    (struct sockaddr *)addr,
                    &addr_size));
}

#endif /* HAS_UNIX_SOCKET */

/* ===========================================================================
 *  Socket Address Related
 */
int socket_address (int sock,
                    struct sockaddr_storage *address)
{
    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_storage);
    return(getpeername(sock, (struct sockaddr *)address, &addr_size));
}

char *socket_str_address (char *buffer,
                          size_t n,
                          const struct sockaddr *address)
{
    const void *addr;

    if (address->sa_family == AF_INET)
        addr = &((const struct sockaddr_in *)address)->sin_addr;
    else
        addr = &((const struct sockaddr_in6 *)address)->sin6_addr;

    if (inet_ntop(address->sa_family, addr, buffer, n) == NULL)
        return(NULL);

    return(buffer);
}

char *socket_str_address_info (char *buffer,
                               size_t n,
                               const struct addrinfo *info)
{
    const void *addr;

    if (info->ai_family == AF_INET)
        addr = &((const struct sockaddr_in *)info->ai_addr)->sin_addr;
    else
        addr = &((const struct sockaddr_in6 *)info->ai_addr)->sin6_addr;

    if (inet_ntop(info->ai_family, addr, buffer, n) == NULL)
        return(NULL);

    return(buffer);
}

int socket_address_is_ipv6 (const struct sockaddr *address) {
    return(address->sa_family != AF_INET);
}

