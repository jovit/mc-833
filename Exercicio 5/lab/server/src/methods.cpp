#include "methods.h"

#define MAXLINE 4096
#define LISTENQ 0
#define MAX_CONNECTIONS 100

using namespace std;

string get_socket_ip(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char address_name[128];

    SocketUtils::Getsockname(fd, (struct sockaddr *)&addr, &len);
    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    stringstream ss;
    ss << address_name;

    return ss.str();
}

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
    string client_ids[MAX_CONNECTIONS];
    int scores[MAX_CONNECTIONS];
    string accepting_game_from[MAX_CONNECTIONS];

    clients[0].fd = listenfd;
    clients[0].events = POLLRDNORM;

    for (int i = 1; i < MAX_CONNECTIONS; i++)
    {
        client_ids[i] = "";
        accepting_game_from[i] = "";
        clients[i].fd = -1;
        scores[i] = 0;
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
                    client_ids[i] = "";
                    accepting_game_from[i] = "";
                    scores[i] = 0;
                    continue;
                }

                string recv(received);
                recv = recv.substr(0, recv.length() - 1);
                cout << recv << endl;

                string command = recv.substr(0, recv.find(" ")); // split string by " ", the first token should be the command
                recv.erase(0, recv.find(" ") + 1);
                string result;

                cout << "received command: " << command << endl;
                if (command == "select_id")
                {
                    cout << "selected user id: " << recv << endl;
                    client_ids[i] = recv; // rest of string contains the user id
                    int amount = 0;
                    for (int j = 0; j < MAX_CONNECTIONS; j++)
                    {
                        if (client_ids[j] == client_ids[i])
                        {
                            amount++;
                        }
                    }
                    if (amount > 1)
                    {
                        cout << "user id already taken: " << recv << endl;
                        result = "fail\n";
                    }
                    else
                    {
                        cout << "success selecting user id: " << recv << endl;
                        result = "ok\n";
                    }
                }
                else if (command == "user_list")
                {
                    cout << "retrieving users" << endl;
                    result = "";
                    for (int j = 0; j < MAX_CONNECTIONS; j++)
                    {
                        if (!client_ids[j].empty() && i != j)
                        {
                            result += "\"" + client_ids[j] + "\"" + " (" + to_string(scores[j]) + ") " + ",";
                        }
                    }
                    if (result.empty())
                    {
                        result = "failed";
                    }
                    else
                    {
                        result = result.substr(0, result.length() - 1);
                    }
                    result += "\n";
                    cout << "retrieved users: " << result;
                }
                else if (command == "play")
                {
                    string player = recv;
                    cout << "request to play " << player << endl;
                    int player_index = -1;

                    for (int j = 0; j < MAX_CONNECTIONS; j++)
                    {
                        if (client_ids[j] == player)
                        {
                            player_index = j;
                        }
                    }

                    if (player_index < 0 || player_index == i)
                    {
                        cout << "player not found" << endl;
                        result = "failed";
                    }
                    else if (!accepting_game_from[player_index].empty())
                    {
                        cout << "player already accepting game" << endl;
                        result = "failed";
                    }
                    else
                    {
                        accepting_game_from[player_index] = client_ids[i];
                        accepting_game_from[i] = client_ids[player_index];

                        cout << "sending confirmation request to user" << endl;
                        string tosend = "play_request\n";
                        SocketUtils::Writen(clients[player_index].fd, tosend.c_str(), tosend.length());
                        result = "success";
                    }

                    result += "\n";
                }
                else if (command == "accept")
                {
                    // do game
                    int player_index = -1;
                    for (int j = 0; j < MAX_CONNECTIONS; j++)
                    {
                        if (client_ids[j] == accepting_game_from[i])
                        {
                            player_index = j;
                        }
                    }

                    string tosend = "game_accepted\n";
                    SocketUtils::Writen(clients[player_index].fd, tosend.c_str(), tosend.length());

                    result = "game_accepted\n";
                }
                else if (command == "deny")
                {
                    int player_index = -1;
                    for (int j = 0; j < MAX_CONNECTIONS; j++)
                    {
                        if (client_ids[j] == accepting_game_from[i])
                        {
                            player_index = j;
                        }
                    }
                    string tosend = "deny\n";
                    SocketUtils::Writen(clients[player_index].fd, tosend.c_str(), tosend.length());
                    accepting_game_from[i] = "";
                    accepting_game_from[player_index] = "";
                    result = "success\n";
                }
                else if (command == "game_port")
                {
                    // do game
                    int player_index = -1;

                    for (int j = 0; j < MAX_CONNECTIONS; j++)
                    {
                        if (client_ids[j] == accepting_game_from[i])
                        {
                            player_index = j;
                        }
                    }

                    string tosend = "game_ip " + get_socket_ip(clients[i].fd) + " " + recv + "\n";
                    SocketUtils::Writen(clients[player_index].fd, tosend.c_str(), tosend.length());

                    result = "ok\n";
                }
                else if (command == "score")
                {
                    int score = atoi(recv.c_str());
                    scores[i] += score;
                    if (scores[i] < 0)
                    {
                        scores[i] = 0;
                    }
                    accepting_game_from[i] = ""; // allow for accepting new games

                    result = "score " + to_string(scores[i]) + "\n";
                }

                SocketUtils::Writen(connfd, result.c_str(), result.length());
                if (--nready <= 0)
                { // check if there are events remaining
                    break;
                }
            }
        }
    }
}