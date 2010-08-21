#ifndef _SOCKPOLL_H_
#define _SOCKPOLL_H_

#include <sys/socket.h>

typedef int (*socket_accept_t) (int sock,
                                const struct sockaddr_storage *address,
                                void *user_data);
typedef int (*socket_read_t)   (int sock,
                                void *user_data);

int     sockpoll_select     (int socket,
                             socket_accept_t accept_f,
                             socket_read_t read_f,
                             void *user_data);

#ifdef HAS_SOCKPOLL_EPOLL
int     sockpoll_epoll      (int socket,
                             socket_accept_t accept_f,
                             socket_read_t read_f,
                             void *user_data);
#endif /* HAS_SOCKPOLL_EPOLL */

#ifdef HAS_SOCKPOLL_KQUEUE
int     sockpoll_kqueue     (int socket,
                             socket_accept_t accept_f,
                             socket_read_t read_f,
                             void *user_data);
#endif /* HAS_SOCKPOLL_KQUEUE */

#endif /* !_SOCKPOLL_H_ */

