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

//todo: faire un fichier utils avec fonction attente reponse serveur par exemple ??


int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "nombre d'arguments : ajouter numéro de port aux arguments du programme \n");
        exit(1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
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
    // todo: addresse
    address_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    int r = connect(fd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
    if (r==-1) {
        perror("connect");
        exit(1);
    }
    if (r==0) {
        char pseudo[MAX_NAME];
        sprintf(pseudo,"client2");
        int nb_sent = send(fd, pseudo, MAX_NAME,0);
        if (nb_sent==-1) {
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
        if (recu > 0) {
            char * max = "MAX";
            int nb_sent = send (fd, max,strlen(max),0);
            if (nb_sent == -1) {
                perror("send");
                exit(1);
            }
            char * buff = malloc(512);
            int curr_size=0;
            int recu = recv(fd, buff, 512,0);
            if (recu == 0) {
                //todo: idk
                sleep(100);
            }
            char * pseudo = malloc(10);
            curr_size += 3;
            memmove(pseudo,buff+curr_size, MAX_NAME);
            curr_size+= MAX_NAME;

            uint32_t ip;
            memmove(&ip,buff+curr_size,sizeof(uint32_t));
            curr_size += sizeof(uint32_t);

            ip = ntohl(ip);
            uint16_t nb;
            memmove(&nb, buff+curr_size,sizeof(uint16_t));
            nb = ntohs(nb);
            // écriture dans sortie standard
            int nb_written = write (1,pseudo,MAX_NAME);
            if (nb_written==-1) {
                perror("write");
                exit(1);
            }
            nb_written = write(1, &ip, sizeof(uint32_t));
            if (nb_written==-1) {
                perror("write");
                exit(1);
            }
            nb_written = write(1, &nb, sizeof(uint16_t));
            if (nb_written==-1) {
                perror("write");
                exit(1);
            }



        }
    }


}