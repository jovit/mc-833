#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

void show_socket_info(int fd)
{
   struct sockaddr_in addr;
   socklen_t len = sizeof(addr);
   char address_name[128];

   if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0)
   {
      perror("getpeername");
      exit(1);
   }

   if (inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name)) < 0) // get string value of the ip address in ipv4 format
   {
      perror("inet_ntop error");
      exit(1);
   }

   printf("\n*****************************\n");
   printf("Local address = %s\n", address_name);
   printf("Local port number = %d\n", ntohs(addr.sin_port));
   printf("*****************************\n");
}

int main(int argc, char **argv)
{
   int sockfd, n;
   char recvline[MAXLINE + 1];
   char error[MAXLINE + 1];
   struct sockaddr_in servaddr;

   if (argc != 3)
   { // make sure that the ip address was passed as an argument
      strcpy(error, "uso: ");
      strcat(error, argv[0]);
      strcat(error, " <IPaddress> <Port>");
      perror(error);
      exit(1);
   }

   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   { // create socket
      perror("socket error");
      exit(1);
   }

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;            // use IPv4
   servaddr.sin_port = htons(atoi(argv[2])); // server port
   if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
   { // convert the argument ip to a binary representation an attribute it to the server
      perror("inet_pton error");
      exit(1);
   }

   if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
   { // establish connection to server
      perror("connect error");
      exit(1);
   }

   show_socket_info(sockfd);

   while ((n = read(sockfd, recvline, MAXLINE)) > 0)
   { // read message line from server
      recvline[n] = 0;
      if (fputs(recvline, stdout) == EOF)
      { // print the received message to stdout
         perror("fputs error");
         exit(1);
      }
   }

   if (n < 0)
   { // if no lines are received from server
      perror("read error");
      exit(1);
   }

   exit(0);
}