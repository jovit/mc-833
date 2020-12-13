#include <iostream>

#include "methods.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    { // make sure that the port was passed as an argument
        std::cerr << "Usage: " << argv[0] << " <Port>"
                  << std::endl;
        exit(1);
    }

    int port = std::atoi(argv[1]);

    int sockfd = init_socket(port);

    accept_connections(sockfd); // loops receiving connections

    return 0;
}