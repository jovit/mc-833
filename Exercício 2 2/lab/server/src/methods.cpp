#include "methods.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <array>

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

void print_connection_info(std::ostream &s, struct sockaddr_in addr, socklen_t len)
{
    char address_name[128];

    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    s << "******************************" << std::endl;
    s << "* Client address = " << address_name << " *" << std::endl;
    s << "* Client port number = " << ntohs(addr.sin_port) << " *" << std::endl;
    s << "******************************" << std::endl
      << std::endl;
}

void accept_connections(int listenfd)
{
    std::array<std::string, 6> commands = {"ls", "pwd", "ls -la", "echo 'lalala' > hello", "cat hello", "rm hello"};

    std::ofstream logfile("logs");
    if (!logfile.is_open())
    {
        std::cout << "Unable to open file" << std::endl;
        exit(1);
    }

    for (;;) // keep accepting connections as they come
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int connfd = SocketUtils::Accept(listenfd, (struct sockaddr *)&addr, &len);

        print_connection_info(logfile, addr, len);
        logfile << "New connection!" << std::endl;

        if (fork() == 0)
        {
            close(listenfd);
            for (const std::string &command : commands)
            {
                char received[MAXLINE];
                SocketUtils::Writen(connfd, command.c_str(), command.length());

                if (SocketUtils::Readall(connfd, received, MAXLINE) == 0)
                {
                    print_connection_info(logfile, addr, len);
                    logfile << "Connection closed!" << std::endl;

                    close(connfd);
                    logfile.close();
                    exit(0);
                }

                // std::cout << "Received response from:" << std::endl;
                logfile << "Received response from:" << std::endl;
                // print_connection_info(std::cout, addr, len);
                print_connection_info(logfile, addr, len);
                // std::cout <<  receive_line << std::endl;
                logfile << received << std::endl;

                SocketUtils::Writen(connfd, received, strlen(received));
            }
            close(connfd);
            exit(0);
        }

        close(connfd); // close connection
    }
}