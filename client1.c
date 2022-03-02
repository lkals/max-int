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

                  if (strcmp(argv[2],"lulu")==0) {
                   co_ip="192.168.70.237";
                  }
                  break;
              default:
                  printf("Usage : ./serveur [port].\nPort par défaut : 4242., ip par défaut : home\n");
                  exit(1);
              }
        printf("ip de co =%s\n",co_ip);

    int fd; //= socket(PF_INET, SOCK_STREAM,0);
    //printf("fd client : %d\n",fd);

    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(port);
    //todo: addresse !!!!
   // inet_aton("192.168.70.237",&address_sock.sin_addr);
    inet_aton(co_ip,&address_sock.sin_addr);

    int r; //= connect(fd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));

    //if (r==0) {
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
                exit(1);
            }
            r = connect(fd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
            if (r==-1) {
                perror("connect");
                exit(1);
            }

            //printf("i:%d\n",i);
            size = sprintf(pseudo, "pseudo%d",i+1);
            if (size > MAX_NAME) {
                perror("pseudo size incorrect");
                exit(1);
            }
            nb_sent = send(fd, pseudo, size, 0);
            //printf("send %s",pseudo);
            if (nb_sent == -1) {
                perror("send");
                exit(1);
            }

            // Il attend ensuite la réponse du serveur de la forme « HELLO␣<pseudo> »
             recu = recv(fd, buffer, (BUFF_SIZE-1),0);
             if (recu == 0) {
                            //todo: idk
                            printf("wait...\n");
                            sleep(1000);
                        }
            if (recu == -1) {
                fprintf( stderr, "ERR:hello pseudo err \n");
                perror("recv");
                exit(1);
            }

            // todo: verifier directement si recu == size(HELLO pseudo) ??
            //if (recu > 0) {

                char model[50];
                size = sprintf(model, "HELLO %s",pseudo);
                //if (size == recu) {
                    if (strcmp(model,buffer)==0) {
                        buffer[recu]='\0';
                        printf("%s\n",buffer);
                        char * debut = "INT";
                        memmove(tmp, debut, strlen(debut));
                        uint16_t rd = rand() % 100;
                        rd = htons(rd);
                        memmove(tmp+strlen(debut),&rd,sizeof(uint16_t));
                        //todo: envoyer pile ce qu'il faut envoyer ou pas ?
                        nb_sent = send (fd, tmp, strlen(debut)+sizeof(uint16_t),0);
                        //printf("%s\n",mess);
                        if (nb_sent==-1) {
                            perror("send");
                            exit(1);
                        }
                        //memset(tmp,0,BUFF_SIZE);

                        // suite du protocole : on doit recevoir INTOK
                        //memset(buffer,0,BUFF_SIZE);
                        //if (nb_sent>0) {
                        memset(buffer,0,BUFF_SIZE);
                                    recu = recv(fd, buffer,(BUFF_SIZE-1),0);
                                    if (recu==-1) {
                                        fprintf( stderr, "ERR:recv intok erreur \n");
                                        perror("recv");
                                        exit(1);
                                    }
                                    if (recu == 0) {
                                     printf("wait...\n");
                                        sleep(1000);
                                    }

                                    if (recu > 0) {
                                        //printf("recu:%d\n",recu);
                                        buffer[recu]='\0';
                                        printf("%s\n",buffer);
                                    }
                                    /*memset(tmp,0,BUFF_SIZE);
                                    memset(buffer,0,BUFF_SIZE);
                                    memset(pseudo,0,MAX_NAME+1);*/
                                    //i++;
                                    //close(fd);

                   // }
                    } /*else {
                        sleep(1000);
                    }*/
                //}
                //sleep(5);
                i++;
                close(fd);
                //sleep(5);
            //}

        }
        free(tmp);
        free(buffer);
        free(pseudo);
   // }
    //close(fd);
    return 0;
}