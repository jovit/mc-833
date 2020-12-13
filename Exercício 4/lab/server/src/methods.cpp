#include "methods.h"

#define MAXLINE 4096
#define LISTENQ 0
#define MAX_CONNECTIONS 100

using namespace std;

// inialize the socket for the server
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

void accept_connections(int listenfd)
{
    struct pollfd clients[MAX_CONNECTIONS];

    clients[0].fd = listenfd;
    clients[0].events = POLLRDNORM;

    for (int i = 1; i < MAX_CONNECTIONS; i++)
    {
        clients[i].fd = -1;
    }

    int maxi = 0;
    for (;;) // keep accepting connections as they come
    {
        cout << "Awaiting for events" << endl;
        int nready = SocketUtils::Poll(clients, maxi + 1, INFTIM);
        cout << "Event received" << endl;

        if (clients[0].revents & POLLRDNORM)
        { // new connection event
            cout << "New connection" << endl;
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            int connfd = SocketUtils::Accept(listenfd, (struct sockaddr *)&addr, &len);
            if (errno == EINTR)
            {
                continue;
            }
            cout << "Connection accepted" << endl;

            bool success = false;
            for (int i = 1; i < MAX_CONNECTIONS; i++)
            {
                if (clients[i].fd < 0)
                {
                    clients[i].fd = connfd; // store connected client descriptor
                    clients[i].events = POLLRDNORM;
                    success = true;
                    if (i > maxi)
                    {
                        maxi = i;
                    }

                    break;
                }
            }

            if (!success)
            {
                cout << "No empty poll available for connection" << endl;
                exit(1);
            }

            if (--nready <= 0)
            { // check if there are events remaining
                continue;
            }
        }

        for (int i = 1; i <= maxi; i++)
        { /* check all clients for data */
            if (clients[i].fd < 0)
            {
                continue;
            }

            if (clients[i].revents & (POLLRDNORM | POLLERR))
            {
                int connfd = clients[i].fd;
                char received[MAXLINE];
                if (SocketUtils::Readall(connfd, received, MAXLINE) <= 0)
                { // there's nothing to read, the connection has been closed

                    cout << "Connection closed" << endl;
                    close(connfd);
                    clients[i].fd = -1;
                    continue;
                }

                SocketUtils::Writen(connfd, received, strlen(received));
                if (--nready <= 0)
                { // check if there are events remaining
                    break;
                }
            }
        }
    }
}