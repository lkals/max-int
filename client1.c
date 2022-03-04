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
                  port= atoi(argv[1]);
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

    printf("ip de co =%s\n",co_ip);

    int fd;
    int r;
    int err=0;
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(port);
    r = inet_aton(co_ip,&address_sock.sin_addr);
    if (r==0) {
        fprintf( stderr, "ERR:adresse ip non valide\n");
        perror("inet_aton");
        exit(1);
    }


    srand(time(NULL));
    int i = 0;
    char * pseudo = calloc(MAX_NAME+1,sizeof(char));
    char * buffer = calloc(BUFF_SIZE,sizeof(char));
    char * tmp = calloc(BUFF_SIZE,sizeof(char));
    int nb_sent;
    int recu;
    int size;
    while (i<5) {
        memset(pseudo,0,MAX_NAME+1);
        memset(buffer,0,BUFF_SIZE);
        memset(tmp,0,BUFF_SIZE);
        fd = socket(PF_INET, SOCK_STREAM,0);
        if (fd==-1) {
            perror("socket");
            err=1;
            break;
        }

        r = connect(fd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
        if (r==-1) {
            perror("connect");
            err=1;
            break;
        }

        size = sprintf(pseudo, "pseudo%d",i+1);
        if (size > MAX_NAME) {
            perror("pseudo size incorrect");
            exit(1);
        }
        nb_sent = send(fd, pseudo, size, 0);
        if (nb_sent == -1) {
            perror("send");
            err=1;
            break;
        }

        // Il attend ensuite la réponse du serveur de la forme « HELLO␣<pseudo> »
        recu = recv(fd, buffer, (BUFF_SIZE-1),0);
        if (recu == 0) {
            sleep(10);
        }
        if (recu == -1) {
            fprintf( stderr, "ERR:hello pseudo err \n");
            perror("recv");
            err=1;
            break;
        }

        char model[50];
        size = sprintf(model, "HELLO %s",pseudo);
        if (strcmp(model,buffer)==0) {
            buffer[recu]='\0';
            printf("%s\n",buffer);
            char * debut = "INT ";
            memmove(tmp, debut, strlen(debut));
            uint16_t rd = rand() % 100;
            rd = htons(rd);
            memmove(tmp+strlen(debut),&rd,sizeof(uint16_t));
            nb_sent = send (fd, tmp, strlen(debut)+sizeof(uint16_t),0);
            if (nb_sent==-1) {
                perror("send");
                err=1;
                break;
            }
            // suite du protocole : on doit recevoir INTOK

            memset(buffer,0,BUFF_SIZE);
            recu = recv(fd, buffer,(BUFF_SIZE-1),0);
            if (recu==-1) {
                fprintf( stderr, "ERR:recv intok erreur \n");
                perror("recv");
                err=1;
                break;
            }
            if (recu == 0) {
                sleep(5);
            }
            if (recu > 0) {
                buffer[recu]='\0';
                printf("%s\n",buffer);
                sleep(2);
            }
        }
        i++;
        close(fd);
    }
    free(tmp);
    free(buffer);
    free(pseudo);
    if (err) {
        close(fd);
        return 1;
    }
    return 0;
}