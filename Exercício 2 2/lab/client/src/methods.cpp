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

int init_socket(char *ip, int port)
{
    struct sockaddr_in servaddr;

    int sockfd = SocketUtils::Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;   // use IPv4
    servaddr.sin_port = htons(port); // server port

    SocketUtils::Inet_pton(AF_INET, ip, &servaddr.sin_addr);

    SocketUtils::Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    return sockfd;
}

void print_local_connection_info(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char address_name[128];

    SocketUtils::Getsockname(fd, (struct sockaddr *)&addr, &len);
    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    std::cout << std::endl
              << "*****************************" << std::endl;
    std::cout << "*** Local connection info ***" << std::endl;
    std::cout << "*****************************" << std::endl;
    std::cout << "* Local address = " << address_name << " *" << std::endl;
    std::cout << "* Local port number = " << ntohs(addr.sin_port) << " *" << std::endl;
    std::cout << "*****************************" << std::endl
              << std::endl;
}

void print_server_connection_info(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char address_name[128];

    SocketUtils::Getpeername(fd, (struct sockaddr *)&addr, &len);
    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    std::cout << std::endl
              << "******************************" << std::endl;
    std::cout << "*** Server connection info ***" << std::endl;
    std::cout << "******************************" << std::endl;
    std::cout << "* Server address = " << address_name << " *" << std::endl;
    std::cout << "* Server port number = " << ntohs(addr.sin_port) << " *" << std::endl;
    std::cout << "******************************" << std::endl
              << std::endl;
}

void start_cli(int fd)
{
    std::string input;
    char receive_line[MAXLINE];

    while (true)
    {
        getline(std::cin, input);
        input = input.append("\n");
        SocketUtils::Writen(fd, input.c_str(), input.length());
        if (SocketUtils::Readline(fd, receive_line, MAXLINE) == 0) {
            std::cout << "Failed to read response from server" << std::endl;
            exit(1);
        }

        std::cout << "Received from server: " << receive_line << std::endl;
    }
}