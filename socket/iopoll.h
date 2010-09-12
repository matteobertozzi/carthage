/* [ iopoll.h ] - I/O (Event Loop) Pool
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

#ifndef _IOPOLL_H_
#define _IOPOLL_H_

#if (!defined(HAS_IOPOLL_SELECT) && \
     !defined(HAS_IOPOLL_KQUEUE) && \
     !defined(HAS_IOPOLL_EPOLL))
    #error No I/O Poll Defined (select/kqueue/epoll)
#endif

#ifdef HAS_IOPOLL_SELECT
#include <sys/select.h>

typedef struct _iopoll_select {
    fd_set fds;
    int    fdmax;
} iopoll_select_t;
#endif /* HAS_IOPOLL_SELECT */

#ifdef HAS_IOPOLL_KQUEUE
#define __IO_KQUEUE_EVENTS      (32)
typedef struct _iopoll_kqueue {
    int kq;
} iopoll_kqueue_t;
#endif /* HAS_IOPOLL_KQUEUE */

#ifdef HAS_IOPOLL_EPOLL
#define __IO_EPOLL_FDS          (32)
typedef struct _iopoll_epoll {
    int epld;
} iopoll_epoll_t;
#endif /* HAS_IOPOLL_EPOLL */

typedef enum _iopoll_backend_type {
    IOPOLL_DEFAULT = 0,
#ifdef HAS_IOPOLL_SELECT
    IOPOLL_SELECT  = 1,
#endif /* HAS_IOPOLL_SELECT */
#ifdef HAS_IOPOLL_KQUEUE
    IOPOLL_KQUEUE  = 2,
#endif /* HAS_IOPOLL_KQUEUE */
#ifdef HAS_IOPOLL_EPOLL
    IOPOLL_EPOLL   = 3,
#endif /* HAS_IOPOLL_EPOLL */
} iopoll_backend_type_t;

typedef union _iopoll_backend {
#ifdef HAS_IOPOLL_SELECT
    iopoll_select_t select;
#endif /* HAS_IOPOLL_SELECT */
#ifdef HAS_IOPOLL_KQUEUE
    iopoll_kqueue_t kqueue;
#endif /* HAS_IOPOLL_KQUEUE */
#ifdef HAS_IOPOLL_EPOLL
    iopoll_epoll_t  epoll;
#endif /* HAS_IOPOLL_EPOLL */
} iopoll_backend_t;

typedef int (*iowrite_t) (void *user_data, int fd);
typedef int (*ioread_t)  (void *user_data, int fd);

typedef struct _iopoll {
    iowrite_t             write_f;           /* Write Handler */
    ioread_t              read_f;            /* Read Handler */
    int *                 is_looping;        /* Continue looping Flag */
    void *                user_data;         /* User Data */
    unsigned int          timeout;           /* msec timeout */
    iopoll_backend_t      backend;           /* I/O Poll Backend */
    iopoll_backend_type_t type;              /* I/O Poll Backend Type */
} iopoll_t;

iopoll_t *  iopoll_alloc    (iopoll_t *iopoll,
                             ioread_t read_f,
                             iowrite_t write_f,
                             iopoll_backend_type_t type,
                             int timeout,
                             int *is_looping,
                             void *user_data);
void        iopoll_free     (iopoll_t *iopoll);
int         iopoll_add      (iopoll_t *iopoll, int fd);
int         iopoll_remove   (iopoll_t *iopoll, int fd);
int         iopoll_loop     (iopoll_t *iopoll);

#endif /* !_IOPOLL_H_ */

