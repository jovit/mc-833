#include "methods.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "SocketUtils.h"

#define MAXLINE 4096
#define LISTENQ 10

int init_socket(int port)
{
    struct sockaddr_in addr;

    int sockfd = SocketUtils::Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;                // use IPv4
    addr.sin_port = htons(port);              // server port
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // accept conections from all IP addresses available

    SocketUtils::Bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    SocketUtils::Listen(sockfd, LISTENQ);

    return sockfd;
}


void print_connection_info(struct sockaddr_in addr, socklen_t len)
{
    char address_name[128];

    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    std::cout << std::endl
              << "******************************" << std::endl;
    std::cout << "*** Received message from: ***" << std::endl;
    std::cout << "******************************" << std::endl;
    std::cout << "* Client address = " << address_name << " *" << std::endl;
    std::cout << "* Client port number = " << ntohs(addr.sin_port) << " *" << std::endl;
    std::cout << "******************************" << std::endl
              << std::endl;
}

void accept_connections(int fd)
{
    char receive_line[MAXLINE];
    for (;;) // keep accepting connections as they come
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int connfd = SocketUtils::Accept(fd, (struct sockaddr *)&addr, &len);

        if (fork() == 0)
        {
            close(fd);
            while (true)
            {
                if (SocketUtils::Readline(connfd, receive_line, MAXLINE) == 0)
                {
                    std::cout << "Client connection closed" << std::endl;
                    close(connfd);
                    exit(0);
                }

                print_connection_info(addr, len);
                std::cout << "Received message: " << receive_line << std::endl;

                SocketUtils::Writen(connfd, receive_line, strlen(receive_line));
            }
        }

        close(connfd); // close connection
    }
}