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

    if (argc != 2) {
        fprintf(stderr, "nombre d'arguments : ajouter numéro de port aux arguments du programme \n");
        exit(1);
    }

    int fd = socket(AF_INET, SOCK_STREAM,0);
    if (fd==-1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;

    int port = atoi(argv[1]);
    if (port == 0) {
        perror("atoi");
        exit(1);
    }
    address_sock.sin_port = htons(port);
    //todo: addresse !!!!
    address_sock.sin_addr.s_addr = htonl(INADDR_ANY);

    int r = connect(fd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
    if (r==-1) {
        perror("connect");
        exit(1);
    }
    if (r==0) {
        srand(time(NULL));
        int i = 0;

        while (i<5) {

            char * pseudo = (char *) malloc(MAX_NAME);
            int size = sprintf(pseudo, "pseudo%d",i+1);
            if (size > MAX_NAME) {
                perror("pseudo size incorrect");
                exit(1);
            }
            int nb_sent = send(fd, pseudo, size, 0);
            if (nb_sent == -1) {
                perror("send");
                exit(1);
            }
            // Il attend ensuite la réponse du serveur de la forme « HELLO␣<pseudo> »
            char * rep = (char *) malloc(100);
            int recu = recv(fd, rep, 99,0);
            if (recu == -1) {
                perror("recv");
                exit(1);
            }
            if (recu == 0) {
                //todo: idk
                sleep(100);
            }
            // todo: verifier directement si recu == size(HELLO pseudo) ??
            if (recu > 0) {
                char model[20];
                int size = sprintf(model, "HELLO %s",pseudo);
                if (size == recu) {
                    if (strcmp(model,rep)==0) {
                        char * mess = (char *) malloc(100);
                        char * debut = "INT";
                        memmove(mess, debut, strlen(debut));
                        uint16_t rd = rand() % 100;
                        rd = htons(rd);
                        memmove(mess+strlen(debut),&rd,sizeof(uint16_t));
                        int nb_sent = send (fd, mess, 100,0);
                        if (nb_sent==-1) {
                            perror("send");
                            exit(1);
                        }
                        free(mess);
                    }
                }
            }
            free(rep);
            free(pseudo);
            i++;
        }
    }
    close(fd);
    return 0;
}