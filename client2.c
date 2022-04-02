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


int main(int argc, char *argv[]) {

   int port=4242;
   char * co_ip="127.0.0.1";
        switch (argc)
           {
           case 1:
               port = 4242;
               break;
           case 2:
               port= atoi(argv[1]);
               break;
           case 3:
               port = atoi(argv[1]);
               if (strcmp(argv[2],"lulu")==0) {
                co_ip="192.168.70.236";
               } else {
                co_ip=argv[2];
               }
               break;
           default:
               printf("Usage : ./serveur [port] [ipv4].\nPort par défaut : 4242., ip par défaut : 127.0.0.1\nExécuter ./serveur [port] lulu \n Pour vous connecter à lulu\n");
               exit(1);
           }
    printf("IP address =%s\n",co_ip);
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd==-1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(port);
    int r = inet_aton(co_ip,&address_sock.sin_addr);
    if (r==0) {
        fprintf( stderr, "ERR: IP address not valid\n");
        perror("inet_aton");
        exit(1);
    }

    r = connect(fd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
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
            close(fd);
            free(pseudo_max);
            free(buffer);
            free(pseudo);
            exit(1);
        }
        // Il attend ensuite la réponse du serveur de la forme « HELLO␣<pseudo> »
        recu = recv(fd, buffer, (BUFF_SIZE-1),0);

        if (recu == -1) {
            perror("recv");
            close(fd);
            free(pseudo_max);
            free(buffer);
            free(pseudo);
            exit(1);
        }
        if (recu == 0) {
            sleep(5);
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
                close(fd);
                free(pseudo_max);
                free(buffer);
                free(pseudo);
                exit(1);
            }
            memset(buffer,0,BUFF_SIZE);
            int curr_size=0;
            recu = recv(fd, buffer, (BUFF_SIZE-1),0);
            if (recu == 0) {
                sleep(5);
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

        } else {
            buffer[recu]='\0';
            printf("%s\n",buffer);
        }
        }
    }
    }
    close(fd);
    free(pseudo_max);
    free(buffer);
    free(pseudo);

    return 0;
}