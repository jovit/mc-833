#include "methods.h"

#define MAXLINE 4096
#define LISTENQ 0

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

// print the connection info from the client
void print_connection_info(ostream &s, struct sockaddr_in addr, socklen_t len)
{
    char address_name[128];

    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    s << "******************************" << endl;
    s << "* Client address = " << address_name << " *" << endl;
    s << "* Client port number = " << ntohs(addr.sin_port) << " *" << endl;
    s << "******************************" << endl
      << endl;
}

// get logfile name given the client details
string logfile_name(struct sockaddr_in addr, socklen_t len)
{
    char address_name[128];

    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));
    auto port = ntohs(addr.sin_port);

    ostringstream os;

    os << address_name << "-" << port << ".log";

    return os.str();
}

void print_connection_closed(ostream &s, struct sockaddr_in addr, socklen_t len)
{
    s << "Connection closed!" << endl;
    print_connection_info(s, addr, len);
}

void sig_chld(int signo) {
    pid_t pid;
    int stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        cout << "child terminated " << pid << endl;
    }
    return;
}

void accept_connections(int listenfd)
{
    array<string, 4> commands = {"ls", "pwd", "ls -la", "echo 'Hello World!'"};

    SocketUtils::Signal(SIGCHLD, sig_chld);

    for (;;) // keep accepting connections as they come
    {
        sleep(5);
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int connfd = SocketUtils::Accept(listenfd, (struct sockaddr *)&addr, &len);
        if (errno == EINTR) {
            continue;
        }

        if (fork() == 0)
        {
            cout << "New connection!" << endl;
            print_connection_info(cout, addr, len);

            close(listenfd);
            while (true)
            {
                for (const string &command : commands)
                {
                    sleep(2); // dont flood the clients stdout
                    char received[MAXLINE];
                    SocketUtils::Writen(connfd, command.c_str(), command.length());

                    if (SocketUtils::Readall(connfd, received, MAXLINE) <= 0)
                    { // there's nothing to read, the connection has been closed
                        print_connection_closed(cout, addr, len);
                        close(connfd);
                        exit(0);
                    }

                    if (strcmp(received, "exit") == 0)
                    { // if the exit command is received
                        print_connection_closed(cout, addr, len);
                        close(connfd);
                        exit(0);
                    }

                    // printing to cout
                    // cout << "Received response from:" << endl;
                    // print_connection_info(cout, addr, len);
                    // cout <<  received << endl;

                    // printing to file
                    // logfile << "Received response from:" << endl;
                    // print_connection_info(logfile, addr, len);
                    // logfile << received << endl;
                    }
            }

            close(connfd);
            exit(0);
        }

        close(connfd); // close connection
    }
}