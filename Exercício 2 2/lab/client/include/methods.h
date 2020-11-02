#pragma once

int init_socket(char* ip, int port);
void print_local_connection_info(int fd);
void print_server_connection_info(int fd);
void start_cli(int fd);
