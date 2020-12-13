#include <iostream>

#include "methods.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    { // make sure that the ip address and port was passed as an argument
        std::cerr << "Usage: " << argv[0] << " <Server IP Address> <Port>"
                  << std::endl;
        exit(1);
    }

    char* ip = argv[1];
    int port = std::atoi(argv[2]);

    int sockfd = init_socket(ip, port);

    start_cli(sockfd);

    return 0;
}