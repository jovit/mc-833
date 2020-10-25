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

int main(int argc, char **argv)
{
   int sockfd, n;
   char recvline[MAXLINE + 1];
   char error[MAXLINE + 1];
   char local_address[128];
   struct sockaddr_in servaddr;
   struct sockaddr_in address;
   socklen_t address_len = sizeof(address);

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

   if (getsockname(sockfd, (struct sockaddr *)&address, &address_len) < 0)
   {
      perror("getsockname");
      exit(1);
   }

   if (inet_ntop(AF_INET, &(address.sin_addr), local_address, sizeof(local_address)) < 0)
   {
      perror("inet_ntop error");
      exit(1);
   }
   printf("Local address = %s\n", local_address);
   printf("Local port number = %d\n", ntohs(address.sin_port));

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