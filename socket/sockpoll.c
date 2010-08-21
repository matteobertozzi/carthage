#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "sockpoll.h"
#include "socket.h"

int sockpoll_select (int socket,
                     socket_accept_t accept_f,
                     socket_read_t read_f,
                     void *user_data)
{
    struct sockaddr_storage addr;
    fd_set fds, rfds;
    int sd, sdmax;
    int client;

    FD_ZERO(&fds);
    FD_SET(socket, &fds);
    sdmax = socket;

    for (;;) {
        memcpy(&rfds, &fds, sizeof(fd_set));
        if (select(sdmax + 1, &rfds, NULL, NULL, NULL) < 0) {
            perror("select()");
            return(-1);
        }

        for (sd = 0; sd <= sdmax; ++sd) {
            if (FD_ISSET(sd, &rfds)) {
                if (sd == socket) {
                    if ((client = socket_tcp_accept(socket, &addr)) < 0)
                        continue;

                    if (accept_f && accept_f(client, &addr, user_data) < 0) {
                        close(client);
                        continue;
                    }

                    FD_SET(client, &fds);
                    if (client > sdmax)
                        sdmax = client;
                } else if (!read_f || read_f(sd, user_data) < 0) {
                    FD_CLR(sd, &fds);
                    close(sd);
                }
            }
        }
    }

    return(0);
}

#ifdef HAS_SOCKPOLL_EPOLL
#include <sys/epoll.h>

int sockpoll_epoll (int socket,
                    socket_accept_t accept_f,
                    socket_read_t read_f,
                    void *user_data)
{
    struct epoll_event events[32];
    struct sockaddr_storage addr;
    struct epoll_event event;
    int n, nfds;
    int client;
    int epld;

    if ((epld = epoll_create(32)) < 0) {
        perror("epoll_create()");
        return(-1);
    }

    fcntl(socket, F_SETFL, O_NONBLOCK);
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = socket;

    if (epoll_ctl(epld, EPOLL_CTL_ADD, socket, &event) < 0) {
        perror("epoll_ctl()");
        close(epld);
        return(-2);
    }

    for (;;) {
        if ((nfds = epoll_wait(epld, events, 32, -1)) < 0) {
            perror("epoll_wait()");
            close(epld);
            return(-3);
        }

        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == socket) {
                if ((client = socket_tcp_accept(socket, &addr)) < 0)
                    continue;

                if (accept_f && accept_f(client, &addr, user_data) < 0) {
                    close(client);
                    continue;
                }

                fcntl(client, F_SETFL, O_NONBLOCK);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client;

                if (epoll_ctl(epld, EPOLL_CTL_ADD, client, &event) < 0) {
                    perror("epoll_ctl()");
                    close(epld);
                    return(-4);
                }
            } else if (!read_f || read_f(events[n].data.fd, user_data) < 0) {
                epoll_ctl(epld, EPOLL_CTL_DEL, events[n].data.fd, NULL);
                close(events[n].data.fd);
            }
        }
    }

    close(epld);

    return(0);
}
#endif /* HAS_SOCKPOLL_EPOLL */

#ifdef HAS_SOCKPOLL_KQUEUE
#include <sys/event.h>

int sockpoll_kqueue (int socket,
                     socket_accept_t accept_f,
                     socket_read_t read_f,
                     void *user_data)
{
    struct sockaddr_storage addr;
    struct kevent events[32];
    struct kevent event;
    int n, nevents;
    int client;
    int kq;

    if ((kq = kqueue()) < 0) {
        perror("kqueue()");
        return(-1);
    }

    memset(&event, 0, sizeof(struct kevent));
    event.ident = socket;
    event.filter = EVFILT_READ;
    event.flags = EV_ADD | EV_ENABLE;

    if (kevent(kq, &event, 1, NULL, 0, NULL) < 0) {
        perror("kevent()");
        close(kq);
        return(-2);
    }

    for (;;) {
        if ((nevents = kevent(kq, NULL, 0, events, 32, NULL)) < 0) {
            perror("kevent()");
            close(kq);
            return(-3);
        }

        for (n = 0; n < nevents; ++n) {
            if (events[n].ident == socket) {
                if ((client = socket_tcp_accept(socket, &addr)) < 0)
                    continue;

                if (accept_f && accept_f(client, &addr, user_data) < 0) {
                    close(client);
                    continue;
                }

                event.ident = client;
                event.filter = EVFILT_READ;
                event.flags = EV_ADD | EV_ENABLE;
                if (kevent(kq, &event, 1, NULL, 0, NULL) < 0) {
                    perror("kevent()");
                    close(kq);
                    return(-4);
                }
            } else if (!read_f || read_f(events[n].ident, user_data) < 0) {
                event.ident = events[n].ident;
                event.filter = EVFILT_READ;
                event.flags = EV_DELETE | EV_DISABLE;
                kevent(kq, &event, 1, NULL, 0, NULL);
                close(events[n].ident);
            }
        }
    }

    close(kq);
    return(0);
}
#endif /* HAS_SOCKPOLL_KQUEUE */

