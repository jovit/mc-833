#pragma once

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <signal.h>
#include <poll.h>

#ifndef INFTIM
#define INFTIM (-1) /* infinite poll timeout */
#endif

typedef void Sigfunc(int);

namespace SocketUtils
{
    int Socket(int family, int type, int flags);
    int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    void Inet_pton(int family, const char *src, void *dst);
    void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    void Listen(int sockfd, int backlog);
    void Getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    void Getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    void Inet_ntop(int family, const void *src, char *dst, socklen_t size);
    ssize_t Readline(int fd, char *ptr, ssize_t maxlen);
    ssize_t Readall(int fd, char *ptr, ssize_t maxlen);
    ssize_t Writen(int fd, const char *ptr, size_t len);
    Sigfunc *Signal(int signo, Sigfunc *func);
    int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout);
    void Shutdown(int fd, int how);
} // namespace SocketUtils