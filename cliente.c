#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include "basic.h"
#include "socket_helper.h"

#define MAXLINE 4096

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

enum role {carrasco = 0, jogador = 1};

void str_cli(FILE *fp, int sockfd);

int main(int argc, char **argv) {
   int    port, sockfd;
   char * ip;
   char   error[MAXLINE + 1];
   struct sockaddr_in servaddr;
   enum   role type = jogador;

   if (argc != 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress, Port>");
      perror(error);
      exit(1);
   }

   ip = argv[1];
   port = atoi(argv[2]);

   sockfd = Socket(AF_INET, SOCK_STREAM, 0);

   servaddr = ClientSockaddrIn(AF_INET, ip, port);

   Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   str_cli(stdin, sockfd);

   close(sockfd);

   exit(0);
}

void str_cli(FILE *fp, int sockfd){
    int maxfdp1, n, stdineof;
    fd_set rset;
    char buf[MAXLINE];

    stdineof = 0;
    FD_ZERO(&rset);

    for( ; ; ){
        if(stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)){
            if((n = read(sockfd, buf, MAXLINE)) == 0){
                if(stdineof == 1)
                    return;
                else {
                    perror("str_cli: server terminated prematurely");
                }
            }

            if(!(strcmp(buf, "exit"))){
                return;
            }
            write(fileno(stdout), buf, n);
        }


        if(FD_ISSET(fileno(fp), &rset)){
            if((n=read(fileno(fp), buf, MAXLINE)) == 0){
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            write(sockfd, buf, n);
        }
    }
}
