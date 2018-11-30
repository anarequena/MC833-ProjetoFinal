#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <arpa/inet.h>
#include "basic.h"
#include "socket_helper.h"

#define LISTENQ 10
#define MAXDATASIZE 4096
#define MAXWORDSIZE 50
#define WELCOME_MESSAGE "Bem vindo ao jogo da forca!\n-----\n1) Iniciar partida simples\n2) Ser carrasco ao iniciar partida\n3) jogar no modo multiplayer\n"

enum role {carrasco = 0, jogador = 1};

char **createDic();
void doit(int connfd, struct sockaddr_in clientaddr);

int main (int argc, char **argv) {
    int listenfd,
        connfd,
        port;
    struct sockaddr_in servaddr;
    char   error[MAXDATASIZE + 1];


    enum   role type = carrasco;
    char   **dictionary;

    if (argc != 2) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <Port> ");
        perror(error);
        exit(1);
    }

    /* uma array de string contendo todas as palavras, do arquivo auxiliar, para
     o jogo */
    if((dictionary = createDic()) == NULL) {
        strcpy(error, "Dicionário inválido");
        exit(1);
    }

    port = atoi(argv[1]);

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    servaddr = ServerSockaddrIn(AF_INET, INADDR_ANY, port);

    Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);


    for ( ; ; ){
        pid_t pid;

        struct sockaddr_in clientaddr;
        socklen_t clientaddr_len = sizeof(clientaddr);

        connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientaddr_len);

        if((pid = fork()) == 0) {
            Close(listenfd);

            doit(connfd, clientaddr);

            Close(connfd);

            exit(0);
        } else {
            waitpid(-1, NULL, 0);
        }
        Close(connfd);
    }

    return(0);
}

char **createDic(int *nwords){
    FILE *fp = fopen("dicionario.txt", "r");
    char **dictionary;
    char *aux;

    if(fp == NULL)
        return NULL;

    while(fscanf(fp, "%c", &aux) != EOF)
        (*nwords)++;

    rewind(fp);
    dictionary = malloc(n_words * sizeof(char *));

    for(int i = 0; i < n_words; i++) {
        dictionary[i] = malloc(MAXWORDSIZE * sizeof(char))
        fscanf(fp, "%c", &dictionary[i]);
    }

    fclose(fp);

    return dictionary;
}

void doit(int connfd, struct sockaddr_in clientaddr, char **dictionary,
          int nwords) {

    char recvline[MAXDATASIZE + 1];
    int n;
    socklen_t remoteaddr_len = sizeof(clientaddr);
    int ncwins = 0;
    int ncgames = 0;
    int ingame = 0;
    int statewords[nwords];
    int chosenindex;
    char chosenword[MAXWORDSIZE];
    int lifes;

    /*  array que indica 0 se a palavra no dicionario nao foi usada ainda com esse
      jogador e 1 se o contrario */
    for(int i = 0; i < nwords; i++)
        statewords[i] = 0;

    /*  enviando o menu */
    write(connfd, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));

    read(connfd, recvline, MAXDATASIZE);

    if(recvline[0] == '1') {

        if(ingame == 0){
            /*  escolhendo uma palavra aleatoriamente */
            srand(time(0));
            while((chosenindex = rand() % nwords) && (statewords[chosenindex] != 0));
            strcpy(chosenword, dictionary[chosenindex]);
            statewords[chosenindex] = 1;
            ingame = 1;
        }
    }
    //write("");

    while ((n = read(connfd, recvline, MAXDATASIZE)) > 0) {
        recvline[n] = 0;

        if(recvline == "1")

        if (getpeername(connfd, (struct sockaddr *) &clientaddr, &remoteaddr_len) == -1) {
            perror("getpeername() failed");
            return;
        }
        write(connfd, recvline, strlen(recvline));
    }
  
}
