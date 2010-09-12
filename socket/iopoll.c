/* [ iopoll.c ] - I/O (Event Loop) Pool
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

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "iopoll.h"

#ifdef HAS_IOPOLL_SELECT
    #include <sys/select.h>
#endif /* HAS_IOPOLL_SELECT */

#ifdef HAS_IOPOLL_KQUEUE
    #include <sys/event.h>
    #include <time.h>
#endif /* HAS_IOPOLL_KQUEUE */

#ifdef HAS_IOPOLL_EPOLL
    #include <sys/epoll.h>
#endif /* HAS_IOPOLL_EPOLL */

#define __iopoll_is_looping(iopoll)                             \
    (((iopoll)->is_looping) ? (*((iopoll)->is_looping)) : 1)

/* ===========================================================================
 *  PRIVATE Methods (Select)
 */
#ifdef HAS_IOPOLL_SELECT
static int __iopoll_select_alloc (iopoll_t *iopoll) {
    iopoll_select_t *io = &(iopoll->backend.select);

    FD_ZERO(&(io->fds));
    io->fdmax = 0;

    return(0);
}

static int __iopoll_select_free (iopoll_t *iopoll) {
    return(0);
}

static int __iopoll_select_add (iopoll_t *iopoll, int fd) {
    iopoll_select_t *io = &(iopoll->backend.select);
    FD_SET(fd, &(io->fds));
    if (fd > io->fdmax)
        io->fdmax = fd;
    return(0);
}

static int __iopoll_select_remove (iopoll_t *iopoll, int fd) {
    iopoll_select_t *io = &(iopoll->backend.select);
    FD_CLR(fd, &(io->fds));
    if (fd == io->fdmax)
        io->fdmax--;
    return(0);
}

static int __iopoll_select (iopoll_t *iopoll) {
    iopoll_select_t *io = &(iopoll->backend.select);
    struct timeval *ptimeout;
    struct timeval timeout;
    fd_set rfds;
    fd_set wfds;
    int x;

    if (iopoll->timeout < 1) {
        timeout.tv_sec = 0;
        timeout.tv_usec = iopoll->timeout * 10;
        ptimeout = &timeout;
    } else {
        ptimeout = NULL;
    }

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    while (__iopoll_is_looping(iopoll)) {
        if (iopoll->read_f != NULL)
            memcpy(&rfds, &(io->fds), sizeof(fd_set));

        if (iopoll->write_f != NULL)
            memcpy(&wfds, &(io->fds), sizeof(fd_set));

        if (iopoll->write_f == NULL)
            x = select(io->fdmax + 1, &rfds, NULL, NULL, ptimeout);
        else if (iopoll->read_f == NULL)
            x = select(io->fdmax + 1, NULL, &wfds, NULL, ptimeout);
        else
            x = select(io->fdmax + 1, &rfds, &wfds, NULL, ptimeout);

        if (x == 0)
            return(1);

        if (x < 0) {
            perror("select()");
            return(-1);
        }

        for (x = 0; x <= io->fdmax && __iopoll_is_looping(iopoll); ++x) {
            if (FD_ISSET(x, &rfds))
                iopoll->read_f(iopoll->user_data, x);

            if (FD_ISSET(x, &wfds))
                iopoll->write_f(iopoll->user_data, x);
        }
    }

    return(0);
}
#endif /* HAS_IOPOLL_SELECT */

/* ===========================================================================
 *  PRIVATE Methods (KQueue)
 */
#ifdef HAS_IOPOLL_KQUEUE
static int __iopoll_kqueue_alloc (iopoll_t *iopoll) {
    iopoll_kqueue_t *io = &(iopoll->backend.kqueue);

    if ((io->kq = kqueue()) < 0) {
        perror("kqueue()");
        return(-1);
    }

    return(0);
}

static int __iopoll_kqueue_free (iopoll_t *iopoll) {
    iopoll_kqueue_t *io = &(iopoll->backend.kqueue);
    close(io->kq);
    return(0);
}

static int __iopoll_kqueue_add (iopoll_t *iopoll, int fd) {
    iopoll_kqueue_t *io = &(iopoll->backend.kqueue);
    struct kevent event;

    event.ident = fd;
    event.flags = EV_ADD | EV_ENABLE;
    event.filter = 0;

    if (iopoll->read_f != NULL)
        event.filter |= EVFILT_READ;

    if (iopoll->write_f != NULL)
        event.filter |= EVFILT_WRITE;

    if (kevent(io->kq, &event, 1, NULL, 0, NULL) < 0) {
        perror("kevent(EV_ADD)");
        return(-1);
    }

    return(0);
}

static int __iopoll_kqueue_remove (iopoll_t *iopoll, int fd) {
    iopoll_kqueue_t *io = &(iopoll->backend.kqueue);
    struct kevent event;

    event.ident = fd;
    event.flags = EV_DELETE | EV_DISABLE;
    event.filter = 0;

    if (iopoll->read_f != NULL)
        event.filter |= EVFILT_READ;

    if (iopoll->write_f != NULL)
        event.filter |= EVFILT_WRITE;

    if (kevent(io->kq, &event, 1, NULL, 0, NULL) < 0) {
        perror("kevent(EV_DELETE)");
        return(-1);
    }

    return(0);
}


static int __iopoll_kqueue (iopoll_t *iopoll) {
    iopoll_kqueue_t *io = &(iopoll->backend.kqueue);
    struct kevent events[__IO_KQUEUE_EVENTS];
    struct timespec *ptimeout;
    struct timespec timeout;
    int i, n;

    if (iopoll->timeout < 1) {
        timeout.tv_sec = 0;
        timeout.tv_nsec = iopoll->timeout * 100;
        ptimeout = &timeout;
    } else {
        ptimeout = NULL;
    }

    while (__iopoll_is_looping(iopoll)) {
        n = kevent(io->kq, NULL, 0, events, __IO_KQUEUE_EVENTS, ptimeout);

        if (n == 0)
            return(1);

        if (n < 0) {
            perror("kevent()");
            return(-1);
        }

        for (i = 0; i < n && __iopoll_is_looping(iopoll); ++i) {
            if (events[i].filter & EVFILT_READ)
                iopoll->read_f(iopoll->user_data, events[i].ident);

            if (events[i].filter & EVFILT_WRITE)
                iopoll->write_f(iopoll->user_data, events[i].ident);
        }
    }

    return(0);
}
#endif /* HAS_IOPOLL_KQUEUE */

/* ===========================================================================
 *  PRIVATE Methods (EPoll)
 */
#ifdef HAS_IOPOLL_EPOLL
static int __iopoll_epoll_alloc (iopoll_t *iopoll) {
    iopoll_epoll_t *io = &(iopoll->backend.epoll);

    if ((io->epld = epoll_create(__IO_EPOLL_FDS)) < 0) {
        perror("epoll_create()");
        return(-1);
    }

    return(0);
}

static int __iopoll_epoll_free (iopoll_t *iopoll) {
    iopoll_epoll_t *io = &(iopoll->backend.epoll);
    close(io->epld);
    return(0);
}

static int __iopoll_epoll_add (iopoll_t *iopoll, int fd) {
    iopoll_epoll_t *io = &(iopoll->backend.epoll);
    struct epoll_event event;

    fcntl(fd, F_SETFL, O_NONBLOCK);
    event.events = EPOLLET;
    event.data.fd = fd;

    if (iopoll->read_f != NULL)
        event.events |= EPOLLIN;

    if (iopoll->write_f != NULL)
        event.events |= EPOLLOUT;

    if (epoll_ctl(io->epld, EPOLL_CTL_ADD, fd, &event) < 0) {
        perror("epoll_ctl(EPOLL_CTL_ADD)");
        return(-1);
    }

    return(0);
}

static int __iopoll_epoll_remove (iopoll_t *iopoll, int fd) {
    iopoll_epoll_t *io = &(iopoll->backend.epoll);

    if (epoll_ctl(io->epld, EPOLL_CTL_DEL, fd, NULL) < 0) {
        perror("epoll_ctl(EPOLL_CTL_DEL)");
        return(-1);
    }

    return(0);
}

static int __iopoll_epoll (iopoll_t *iopoll) {
    iopoll_epoll_t *io = &(iopoll->backend.epoll);
    struct epoll_event events[__IO_EPOLL_FDS];
    int i, n;

    while (__iopoll_is_looping(iopoll)) {
        n = epoll_wait(io->epld, events, __IO_EPOLL_FDS, iopoll->timeout);

        if (n == 0)
            return(1);

        if (n < 0) {
            perror("epoll_wait()");
            return(-1);
        }

        for (i = 0; i < n && __iopoll_is_looping(iopoll); ++i) {
            if (events[i].events & EPOLLIN)
                iopoll->read_f(iopoll->user_data, events[i].data.fd);

            if (events[i].events & EPOLLOUT)
                iopoll->write_f(iopoll->user_data, events[i].data.fd);
        }
    }

    return(0);
}
#endif /* HAS_IOPOLL_EPOLL */

/* ===========================================================================
 *  PUBLIC Methods
 */
iopoll_t *iopoll_alloc (iopoll_t *iopoll,
                        ioread_t read_f,
                        iowrite_t write_f,
                        iopoll_backend_type_t type,
                        int timeout,
                        int *is_looping,
                        void *user_data)
{
    iopoll->read_f = read_f;
    iopoll->write_f = write_f;
    iopoll->timeout = (timeout <= 0) ? -1 : timeout;
    iopoll->is_looping = is_looping;
    iopoll->user_data = user_data;

    if (type == IOPOLL_DEFAULT) {
#if   defined(HAS_IOPOLL_EPOLL)
        type = IOPOLL_EPOLL;
#elif defined(HAS_IOPOLL_KQUEUE)
        type = IOPOLL_KQUEUE;
#elif defined(HAS_IOPOLL_SELECT)
        type = IOPOLL_SELECT;
#else
        return(NULL);
#endif
    }

    switch ((iopoll->type = type)) {
#ifdef HAS_IOPOLL_SELECT
        case IOPOLL_SELECT:
            if (__iopoll_select_alloc(iopoll))
                return(NULL);
            break;
#endif /* HAS_IOPOLL_SELECT */
#ifdef HAS_IOPOLL_KQUEUE
        case IOPOLL_KQUEUE:
            if (__iopoll_kqueue_alloc(iopoll))
                return(NULL);
            break;
#endif /* HAS_IOPOLL_KQUEUE */
#ifdef HAS_IOPOLL_EPOLL
        case IOPOLL_EPOLL:
            if (__iopoll_epoll_alloc(iopoll))
                return(NULL);
            break;
#endif /* HAS_IOPOLL_EPOLL */
        default:
            break;
    }

    return(iopoll);
}

void iopoll_free (iopoll_t *iopoll) {
    switch (iopoll->type) {
#ifdef HAS_IOPOLL_SELECT
        case IOPOLL_SELECT:
            __iopoll_select_free(iopoll);
            break;
#endif /* HAS_IOPOLL_SELECT */
#ifdef HAS_IOPOLL_KQUEUE
        case IOPOLL_KQUEUE:
            __iopoll_kqueue_free(iopoll);
            break;
#endif /* HAS_IOPOLL_KQUEUE */
#ifdef HAS_IOPOLL_EPOLL
        case IOPOLL_EPOLL:
            __iopoll_epoll_free(iopoll);
            break;
#endif /* HAS_IOPOLL_EPOLL */
        default:
            break;
    }
}

int iopoll_add (iopoll_t *iopoll, int fd) {
    switch (iopoll->type) {
#ifdef HAS_IOPOLL_SELECT
        case IOPOLL_SELECT: return(__iopoll_select_add(iopoll, fd));
#endif /* HAS_IOPOLL_SELECT */
#ifdef HAS_IOPOLL_KQUEUE
        case IOPOLL_KQUEUE: return(__iopoll_kqueue_add(iopoll, fd));
#endif /* HAS_IOPOLL_KQUEUE */
#ifdef HAS_IOPOLL_EPOLL
        case IOPOLL_EPOLL:  return(__iopoll_epoll_add(iopoll, fd));
#endif /* HAS_IOPOLL_EPOLL */
        default:
            break;
    }

    return(1);
}

int iopoll_remove (iopoll_t *iopoll, int fd) {
    switch (iopoll->type) {
#ifdef HAS_IOPOLL_SELECT
        case IOPOLL_SELECT: return(__iopoll_select_remove(iopoll, fd));
#endif /* HAS_IOPOLL_SELECT */
#ifdef HAS_IOPOLL_KQUEUE
        case IOPOLL_KQUEUE: return(__iopoll_kqueue_remove(iopoll, fd));
#endif /* HAS_IOPOLL_KQUEUE */
#ifdef HAS_IOPOLL_EPOLL
        case IOPOLL_EPOLL:  return(__iopoll_epoll_remove(iopoll, fd));
#endif /* HAS_IOPOLL_EPOLL */
        default:
            break;
    }

    return(1);
}

int iopoll_loop (iopoll_t *iopoll) {
    switch (iopoll->type) {
#ifdef HAS_IOPOLL_SELECT
        case IOPOLL_SELECT: return(__iopoll_select(iopoll));
#endif /* HAS_IOPOLL_SELECT */
#ifdef HAS_IOPOLL_KQUEUE
        case IOPOLL_KQUEUE: return(__iopoll_kqueue(iopoll));
#endif /* HAS_IOPOLL_KQUEUE */
#ifdef HAS_IOPOLL_EPOLL
        case IOPOLL_EPOLL:  return(__iopoll_epoll(iopoll));
#endif /* HAS_IOPOLL_EPOLL */
        default:
            break;
    }

    return(1);
}

