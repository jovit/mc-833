#include "methods.h"

#define MAXLINE 4096

using namespace std;

// initialize the clients socket
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

bool check_exit_command()
{
    string input;

    while (true)
    {
        getline(cin, input);
        if (input == "exit")
        {
            return true;
        }
    }

    return false;
}

void start_cli(int fd)
{
    struct pollfd polls[2]; // array of poll structures, where 0 is the server poll and 1 is the stdin

    polls[0].fd = fd;
    polls[0].events = POLLRDNORM;

    polls[1].fd = fileno(stdin);
    polls[1].events = POLLRDNORM;

    string input;
    // cout << "starting" << endl;
    for (;;)
    {
        // cout << "polling" << endl;
        int nready = SocketUtils::Poll(polls, 2, INFTIM);
        // cout << "ready " << nready << endl;

        if (polls[0].revents & (POLLRDNORM | POLLERR)) // server event
        {
            char received_line[MAXLINE];
            // cout << "server " << endl;

            if (SocketUtils::Readall(fd, received_line, MAXLINE) == 0)
            {
                close(fd);
                exit(0);
            }
            string received_str = string(received_line);
            cout << received_str;
            if (--nready <= 0)
                continue; /* no more readable descriptors */
        }

        if (polls[1].revents & (POLLRDNORM | POLLERR)) // stdin event
        {
            // cout << "stdin " << endl;
            string input;
            getline(cin, input);
            if (!cin.eof()) { // only add the endline if not end of file
                input += "\n";
            }
        
            // cout << "read stdin " << input << endl;
            SocketUtils::Writen(fd, input.c_str(), input.length());
            
            if (cin.eof()) {
                // cout << "EOF" << endl;
                SocketUtils::Shutdown(fd, SHUT_WR); // shutdown writing to the server
                polls[1].fd = -1; // disble stdin polling
            }

            if (--nready <= 0)
                continue; /* no more readable descriptors */
        }
    }

    close(fd);
    exit(0);
}