#pragma once

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <future>
#include <thread>
#include <chrono>
#include "poll.h"

#include "SocketUtils.h"

int init_socket(char *ip, int port);
void print_local_connection_info(int fd);
void print_server_connection_info(int fd);
void start_cli(int fd);
