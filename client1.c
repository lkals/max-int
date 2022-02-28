#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#define MAX_NAME 10


int main(int argc, char *argv[]) {

   int port;
        switch (argc)
           {
           case 1:
               port = 4242;
               break;
           case 2:
               port= atoi(argv[1]);
               break;
           default:
               printf("Usage : ./serveur [port].\nPort par défaut : 4242.\n");
               exit(1);
           }

    int fd = socket(PF_INET, SOCK_STREAM,0);
    //printf("fd client : %d\n",fd);
    if (fd==-1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(port);
    //todo: addresse !!!!
    inet_aton("127.0.0.1",&address_sock.sin_addr);

    int r = connect(fd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
    if (r==-1) {
        perror("connect");
        exit(1);
    }
    if (r==0) {
        srand(time(NULL));
        int i = 0;
        char * pseudo = (char *) malloc(MAX_NAME+1);
        while (i<5) {
            printf("i :%d\n",i);

            int size = sprintf(pseudo, "pseudo%d",i+1);
            if (size > MAX_NAME) {
                perror("pseudo size incorrect");
                exit(1);
            }
            int nb_sent = send(fd, pseudo, size, 0);
            //printf("send %s",pseudo);
            if (nb_sent == -1) {
                perror("send");
                exit(1);
            }

            // Il attend ensuite la réponse du serveur de la forme « HELLO␣<pseudo> »
            char * buffer = (char *) malloc(100);
            char * tmp =(char *) malloc(100);;
            int recu = recv(fd, buffer, 99,0);
             if (recu == 0) {
                            //todo: idk
                            sleep(1000);
                        }
            if (recu == -1) {
                perror("recv");
                exit(1);
            }

            // todo: verifier directement si recu == size(HELLO pseudo) ??
            if (recu > 0) {
                printf("%s\n",buffer);
                char model[20];
                int size = sprintf(model, "HELLO %s",pseudo);
                if (size == recu) {
                    if (strcmp(model,buffer)==0) {
                        memset(tmp,0,100);
                        char * debut = "INT";
                        memmove(tmp, debut, strlen(debut));
                        uint16_t rd = rand() % 100;
                        rd = htons(rd);
                        memmove(tmp+strlen(debut),&rd,sizeof(uint16_t));
                        int nb_sent = send (fd, tmp, 100,0);
                        //printf("%s\n",mess);
                        if (nb_sent==-1) {
                            perror("send");
                            exit(1);
                        }
                        memset(tmp,0,100);
                    }
                }
            }
            memset(buffer,0,100);
            recu = recv(fd, buffer,99,0);
            if (recu == 0) {
                sleep(1000);
            }
            if (recu >0) {
                printf("%s\n",buffer);
            }
            memset(buffer,0,100);
            memset(pseudo,0,MAX_NAME+1);
            printf("incr\n");
            i++;
            close(fd);
        }
    }
    close(fd);
    return 0;
}