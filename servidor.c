#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
#define BEGIN_GAME "A partida de jogo da forca comecou!\n-----\n"
#define PLAY_AGAIN "Deseja jogar outra partida, digite \"SIM\"\n"
#define WIN_MESSAGE "Você adivinhou a palavra!\n"
#define INVALID_MESSAGE "Letra invalida\n"

enum role {carrasco = 0, jogador = 1};

char **createDic(int *nwords);
void doit(int connfd, struct sockaddr_in clientaddr, char **dictionary, int nwords);

int main (int argc, char **argv) {
    int listenfd,
        connfd,
        port,
        zero = 0;
    struct sockaddr_in servaddr;
    char   error[MAXDATASIZE + 1];
    int *nwords = &zero;

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
    if((dictionary = createDic(nwords)) == NULL) {
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

            doit(connfd, clientaddr, dictionary, (*nwords));

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
    char aux[MAXDATASIZE];

    if(fp == NULL)
        return NULL;

    while(fscanf(fp, "%s ", aux) != EOF){
        (*nwords)++;
    }

    rewind(fp);
    dictionary = malloc((*nwords) * sizeof(char *));

    for(int i = 0; i < (*nwords); i++) {
        dictionary[i] = malloc(MAXWORDSIZE * sizeof(char));
        fscanf(fp, "%s", dictionary[i]);
    }

    fclose(fp);

    return dictionary;
}

void doit(int connfd, struct sockaddr_in clientaddr, char **dictionary,
          int nwords) {

    char recvline[MAXDATASIZE + 1],
        lostmessage[MAXDATASIZE],
        beginmessage[MAXDATASIZE],
        trymessage[MAXDATASIZE];
    int n;
    socklen_t remoteaddr_len = sizeof(clientaddr);
    int ncwins = 0, ncgames = 0, ingame = 0;
    int statewords[nwords];
    int chosenindex;
    char chosenword[MAXWORDSIZE],
        aux[MAXWORDSIZE],
        word[MAXWORDSIZE],
        alphabet[123],
        letter;
    int lifes = 6, win = 0, begin = 0;

    /*  array que indica 0 se a palavra no dicionario nao foi usada ainda com esse
      jogador e 1 se o contrario */
    for(int i = 0; i < nwords; i++)
        statewords[i] = 0;

    /*  enviando o menu */
    write(connfd, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));


    while ((n = read(connfd, recvline, MAXDATASIZE)) > 0) {
        recvline[n] = 0;

        if((ingame == 2)&&(!(strcmp(recvline, "SIM\n")))){
            doit(connfd, clientaddr, dictionary, nwords);
            return;
        } else if(ingame == 2){
            write(connfd, "exit", 5);
        }

        if(recvline[0] == '1') {
            if(ingame == 0){
                /*  escolhendo uma palavra aleatoriamente */
                srand(time(NULL));
                while((chosenindex = rand() % nwords) && (statewords[chosenindex] != 0));
                strcpy(chosenword, dictionary[chosenindex]);
                statewords[chosenindex] = 1;
                printf("%d %s\n", chosenindex, chosenword);
                ingame = 1;

                /* display do numero de letras da palavra */
                for(int i = 0; i < strlen(chosenword); i++){
                    aux[i*2] = '_';
                    aux[i*2 + 1] = ' ';
                }
                sprintf(word, "%s\n", aux);

                /* começa a partida */
                write(connfd, BEGIN_GAME, strlen(BEGIN_GAME));
                int len = sprintf(beginmessage, "Você possui %d vidas.\nA palavra possui %ld caracteres\n", lifes, strlen(chosenword));
                write(connfd, beginmessage, len);
                write(connfd, word, 2*strlen(chosenword)+1);

                // flag de inicio de jogo
                begin = 1;
            }
        }

        if(ingame == 1){
            /* verifica se o caracter eh uma letra ou nao */
            letter = toupper(recvline[0]);

            if((letter < 65)||(letter > 90)){
                if(begin == 0)
                    write(connfd, INVALID_MESSAGE, strlen(INVALID_MESSAGE));
            } else if(begin == 0){
                // flag de letra acertada
                win = 0;

                if(alphabet[letter]==1){
                    int len = sprintf(trymessage, "A letra '%c' ja foi utilizada\n", letter);
                    write(connfd, trymessage, len);
                    write(connfd, word, 2*strlen(chosenword)+1);
                } else {

                    for(int i = 0; i < strlen(chosenword); i++){
                        if(letter == chosenword[i]){
                            word[i*2] = chosenword[i];
                            alphabet[letter] = 1;
                            win = 1;
                            ncwins += 1;
                        }
                    }

                    if(ncwins == strlen(chosenword)){
                        write(connfd, WIN_MESSAGE, strlen(WIN_MESSAGE));
                        write(connfd, PLAY_AGAIN, strlen(PLAY_AGAIN));
                        ingame = 2;
                    }

                    if(win == 1){
                        write(connfd, word, 2*strlen(chosenword)+1);
                    } else {
                        if(lifes == 1){
                            int len = sprintf(lostmessage, "A palavra correta era %s, você perdeu!\n", chosenword);
                            write(connfd, lostmessage, len);
                            write(connfd, PLAY_AGAIN, strlen(PLAY_AGAIN));
                            ingame = 2;
                        } else {
                            lifes -= 1;
                            alphabet[letter] = 1;
                            int len = sprintf(trymessage, "A palavra não tem nenhuma letra '%c'.\nVocê agora possui %d vidas.\n", letter, lifes);
                            write(connfd, trymessage, len);
                            write(connfd, word, 2*strlen(chosenword)+1);
                        }
                    }
                }
            }
            begin = 0;
        }

        if (getpeername(connfd, (struct sockaddr *) &clientaddr, &remoteaddr_len) == -1) {
            perror("getpeername() failed");
            return;
        }

    }

}
