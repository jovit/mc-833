#pragma once

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <sstream>
#include <sys/wait.h>

#include "SocketUtils.h"

int init_socket(int port);
void accept_connections(int fd);
