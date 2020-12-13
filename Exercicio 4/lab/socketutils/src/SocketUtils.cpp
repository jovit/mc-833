#include "SocketUtils.h"

using namespace SocketUtils;

int SocketUtils::Poll(pollfd *fdarray, unsigned long nfds, int timeout)
{
    int n;

    if ((n = poll(fdarray, nfds, timeout)) < 0)
        perror("poll error");

    return (n);
}

void SocketUtils::Shutdown(int fd, int how)
{
    if (shutdown(fd, how) < 0)
        perror("shutdown error");
}

Sigfunc *SocketUtils::Signal(int signo, Sigfunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM)
    {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */
#endif
    }
    else
    {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART; /* SVR4, 4.4BSD */
#endif
    }

    if (sigaction(signo, &act, &oact) < 0)
    {
        return (SIG_ERR);
    }
    return (oact.sa_handler);
}

int SocketUtils::Socket(int family, int type, int flags)
{
    int sockfd;
    if ((sockfd = socket(family, type, flags)) < 0) // create socket
    {
        perror("socket error");
        exit(1);
    }

    return sockfd;
}

int SocketUtils::Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int connfd;
    if ((connfd = accept(sockfd, addr, addrlen)) == -1) // // accept pending connection
    {
        perror("accept error");
        exit(1);
    }

    return connfd;
}

void SocketUtils::Inet_pton(int family, const char *src, void *dst)
{
    if (inet_pton(family, src, dst) <= 0) // convert the argument ip to a binary representation an attribute it to the server
    {
        perror("inet_pton error");
        exit(1);
    }
}

void SocketUtils::Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (connect(sockfd, addr, addrlen) < 0) // establish connection to server
    {
        perror("connect error");
        exit(1);
    }
}

void SocketUtils::Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (bind(sockfd, addr, addrlen) < 0) // establish connection to server
    {
        perror("bind error");
        exit(1);
    }
}

void SocketUtils::Listen(int sockfd, int backlog)
{
    if (listen(sockfd, backlog) == -1) // put socket on passive mode to accept incoming connections
    {
        perror("listen error");
        exit(1);
    }
}

void SocketUtils::Getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    if (getsockname(sockfd, addr, addrlen) < 0)
    {
        perror("getsockname error");
        exit(1);
    }
}

void SocketUtils::Inet_ntop(int family, const void *src, char *dst, socklen_t size)
{
    if (inet_ntop(family, src, dst, size) == nullptr) // get string value of the ip address in ipv4 format
    {
        perror("inet_ntop error");
        exit(1);
    }
}

void SocketUtils::Getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    if (getpeername(sockfd, addr, addrlen) < 0)
    {
        perror("getsockname error");
        exit(1);
    }
}

ssize_t readline(int fd, char *vptr, ssize_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++)
    {
        if ((rc = read(fd, &c, 1)) == 1)
        {                        
            *ptr++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
        {
            if (n == 1)
                return (0); /* EOF, no data read */
            else
                break; /* EOF, some data was read */
        }
        else
            return (-1); /* error */
    }

    *ptr = 0;
    return (n);
}

ssize_t SocketUtils::Readline(int fd, char *ptr, ssize_t maxlen)
{
    ssize_t n;

    if ((n = readline(fd, ptr, maxlen)) == -1)
        perror("readline error");
    return (n);
}

ssize_t readall(int fd, char *vptr, ssize_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++)
    {
        if ((rc = read(fd, &c, 1)) == 1)
        {
            *ptr++ = c;
            if (c == '\0')
            {
                break;
            }
        }
        else if (rc == 0)
        {
            if (n == 1)
                return (0); /* EOF, no data read */
            else
                break; /* EOF, some data was read */
        }
        else
            return (-1); /* error */
    }

    *ptr = 0;
    return (n);
}

// reads until \0
ssize_t SocketUtils::Readall(int fd, char *ptr, ssize_t maxlen)
{
    ssize_t n;

    if ((n = readall(fd, ptr, maxlen)) == -1)
        perror("readall error");
    return (n);
}

ssize_t SocketUtils::Writen(int fd, const char *ptr, size_t len)
{
    size_t bytes_sent = (0);
    size_t bytes_to_send = len + 1;

    while (bytes_to_send > 0)
    {
        if ((bytes_sent = write(fd, ptr, bytes_to_send)) <= 0)
        {
            perror("write error");
            return (bytes_sent); // failure
        }

        bytes_to_send -= bytes_sent;
        ptr += bytes_sent;
    }

    return (len);
}
