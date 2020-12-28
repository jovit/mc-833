#include "methods.h"
#include "tictactoe.h"

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

void start_cli(int fd)
{
    TicTacToe::Game game;
    game.init_udp_socket();
    
    struct pollfd polls[2]; // array of poll structures, where 0 is the server poll and 1 is the stdin

    polls[0].fd = fd;
    polls[0].events = POLLRDNORM;

    polls[1].fd = fileno(stdin);
    polls[1].events = POLLRDNORM;

    string input;
    game.input_id();
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

            string to_send = game.handle_server_response(received_str.substr(0, received_str.length() - 1));

            if (!to_send.empty())
            {
                to_send += "\n";
                SocketUtils::Writen(fd, to_send.c_str(), to_send.length());
            }

            if (--nready <= 0)
                continue; /* no more readable descriptors */
        }

        if (polls[1].revents & (POLLRDNORM | POLLERR)) // stdin event
        {
            // cout << "stdin " << endl;
            string input;
            getline(cin, input);

            string to_send = game.handle_stdin(input);

            // cout << "read stdin " << input << endl;
            if (!to_send.empty())
            {
                to_send += "\n";
                SocketUtils::Writen(fd, to_send.c_str(), to_send.length());
            }

            // if (cin.eof()) {
            //     // cout << "EOF" << endl;
            //     SocketUtils::Shutdown(fd, SHUT_WR); // shutdown writing to the server
            //     polls[1].fd = -1; // disble stdin polling
            // }

            if (--nready <= 0)
                continue; /* no more readable descriptors */
        }
    }

    close(fd);
    exit(0);
}