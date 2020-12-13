#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTENQ 10
#define MAXDATASIZE 100

void show_server_socket_info(int fd)
{
   struct sockaddr_in addr;
   socklen_t len = sizeof(addr);
   char address_name[128];

   if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0) // get address info of passed filed descriptor
   {
      perror("getpeername");
      exit(1);
   }

   if (inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name)) < 0) // get the IPv4 string value of the address
   {
      perror("inet_ntop error");
      exit(1);
   }

   printf("\n*****************************\n");
   printf("Server address = %s\n", address_name);
   printf("Server port number = %d\n", ntohs(addr.sin_port));
   printf("*****************************\n");
}

void show_peer_info(int fd)
{
   struct sockaddr_in addr;
   socklen_t len = sizeof(addr);
   char address_name[128];

   if (getpeername(fd, (struct sockaddr *)&addr, &len) < 0) // get address info of passed filed descriptor
   {
      perror("getpeername");
      exit(1);
   }

   if (inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name)) < 0) // get the IPv4 string value of the address
   {
      perror("inet_ntop error");
      exit(1);
   }

   printf("\n-----------------------------\n");
   printf("Client address = %s\n", address_name);
   printf("Client port number = %d\n", ntohs(addr.sin_port));
   printf("-----------------------------\n");
}

int main(int argc, char **argv)
{
   int listenfd, connfd;
   struct sockaddr_in servaddr;
   char buf[MAXDATASIZE];
   time_t ticks;

   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) // create tcp ipv4 socket
   {
      perror("socket");
      exit(1);
   }

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;                // IPv4
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // accept conections from all IP addresses available
   servaddr.sin_port = 0;                        // will be attributed a random port on bind
   // servaddr.sin_port = htons(8585); // start server on fixed port

   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) // bind address configuration do created socket
   {
      perror("bind");
      exit(1);
   }

   if (listen(listenfd, LISTENQ) == -1) // put socket on passive mode to accept incoming connections
   {
      perror("listen");
      exit(1);
   }

   show_server_socket_info(listenfd);

   for (;;) // keep accepting connections as they come
   {
      if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1) // // accept pending connection
      {
         perror("accept");
         exit(1);
      }

      show_peer_info(connfd);
      ticks = time(NULL);                                     // get currenty timestamp
      snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks)); // write current timestamp string to buffer
      write(connfd, buf, strlen(buf));                        // send buffer to the accepted socket connection

      close(connfd); // close connection
   }

   return (0);
}