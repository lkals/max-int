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
#define BUFF_SIZE 256

//todo: faire un fichier utils avec fonction attente reponse serveur par exemple ??


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

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd==-1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;

    address_sock.sin_port = htons(port);
    // todo: addresse : lulu ou host ?
    //inet_aton("192.168.70.237",&address_sock.sin_addr);
     inet_aton("127.0.0.1",&address_sock.sin_addr);

    int r = connect(fd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
    if (r==-1) {
        perror("connect");
        exit(1);
    }
    char * pseudo = calloc(MAX_NAME+1,sizeof(char));
    char * buffer = calloc(BUFF_SIZE,sizeof(char));
    char * pseudo_max = calloc(MAX_NAME+1,sizeof(char));


    int nb_sent;
    int recu;
    int size;
    if (r==0) {

        size = sprintf(pseudo,"client2");
        nb_sent = send(fd, pseudo, size,0);
        if (nb_sent==-1) {
            perror("send");
            exit(1);
        }
        // Il attend ensuite la réponse du serveur de la forme « HELLO␣<pseudo> »
        recu = recv(fd, buffer, (BUFF_SIZE-1),0);

        if (recu == -1) {
            perror("recv");
            exit(1);
        }
        if (recu == 0) {
            //todo: idk
            sleep(1000);
        }
        if (recu > 0) {

            // verifie qu'on a reçu la réponse de format « HELLO␣<pseudo> »

            char model[20];
            size = sprintf(model, "HELLO %s",pseudo);
            if (strcmp(model,buffer)==0) {
                buffer[recu]='\0';
                printf("%s\n",buffer);

            char * max = "MAX";
            nb_sent = send (fd, max,strlen(max),0);
            if (nb_sent == -1) {
                perror("send");
                exit(1);
            }
            memset(buffer,0,BUFF_SIZE);
            int curr_size=0;
            recu = recv(fd, buffer, (BUFF_SIZE-1),0);
            if (recu == 0) {
                //todo: idk
                sleep(1000);
            }

            if (strcmp(buffer,"NOP")!=0) {
            curr_size += 3;
            memmove(pseudo_max,buffer+curr_size, MAX_NAME);
            curr_size+= MAX_NAME;

            uint32_t ip;
            memmove(&ip,buffer+curr_size,sizeof(uint32_t));
            curr_size += sizeof(uint32_t);

            ip = ntohl(ip);
            uint16_t nb;
            memmove(&nb, buffer+curr_size,sizeof(uint16_t));
            nb = ntohs(nb);
            // écriture dans sortie standard

            pseudo_max[MAX_NAME]='\0';
            printf("pseudo:%s\n",pseudo_max);
            // affichage de l'adresse ip:
            struct in_addr addr;
            addr.s_addr=ip;
            char * ipv4 = inet_ntoa(addr);
            printf("ip:%s\n",ipv4);
            printf("nb:%d\n",nb);
            /*int nb_written = write (1,pseudo_max,MAX_NAME);
            if (nb_written==-1) {
                perror("write");
                exit(1);
            }
            printf("\nip:\n");
            nb_written = write(1, &ip, sizeof(uint32_t));
            if (nb_written==-1) {
                perror("write");
                exit(1);
            }
            printf("\nnb:\n");
            nb_written = write(1, &nb, sizeof(uint16_t));
            if (nb_written==-1) {
                perror("write");
                exit(1);
            }*/
        } else {
            buffer[recu]='\0';
            printf("%s\n",buffer);
        }
        }
    }
    }
    free(pseudo_max);
    free(buffer);
    free(pseudo);
    close(fd);
    return 0;
}