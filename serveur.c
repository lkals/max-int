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


typedef struct {
    int fd;
    struct sockaddr_in * addr;
    char * pseudo;
    int nb;
} cli_info;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// todo: ça devrait ê un pointeur ?
cli_info cli_max;
// booléen indiquant si on a reçu un entier ou pas
int reception;

// TODO: tester sur lulu et sur d'autres ordis
// TODO: faire un goto error si temps
// todo: define constante buffer
//todo: utiliser les fonctions mem (memmove plutot que mempcpy) pour envoyer des suites d'octets (envoyer des int en big endian et non pas des string

void * maxint(void *arg);

int main(int argc, char *argv[]) {
    cli_max.nb= 0;
    cli_info *cli;
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
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd==-1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(port);
    address_sock.sin_addr.s_addr=htonl(INADDR_ANY);

    int r = bind(sockfd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
    if (r==-1) {
        perror("bind");
        exit(1);
    }
    if (r==0) {
        r = listen(sockfd,0);
        //if (r==0) {

            while (1) {
                struct sockaddr_in caller;
                socklen_t size = sizeof(caller);
                int * sockcli = (int *)malloc(sizeof(int));
                cli = malloc (sizeof(cli_info));
                *sockcli = accept(sockfd, (struct sockaddr *)&caller, &size);
                cli->fd = *sockcli;
                cli->addr = &caller;
                cli->nb = 0;
                cli->pseudo = NULL;
                if (cli>=0 && sockcli>0) {
                    pthread_t th;
                    pthread_create (&th, NULL, maxint,cli);
                }
            }
       // }
        // todo: faire les free et close nécessaires
    } else {
        perror("Can't start server");
        return 1;
    }
    return 0;
}

void * maxint(void *arg) {

    cli_info * cli = ((cli_info *)arg);
    cli->pseudo =malloc (MAX_NAME+1);
    int fd = (cli->fd);

    // on attend le message du client <pseudo>
    int recu = recv(fd, cli->pseudo, MAX_NAME,0);
    if (recu==-1) {
        perror("recv");
        exit(1);
    }
    if (recu==0) {
        close(fd);
        return NULL;
    }


    char *buffer = malloc(BUFF_SIZE*sizeof(char));
    char * tmp = malloc(BUFF_SIZE*sizeof(char));
    if (recu >0) {
    //todo: attention au charactère final
cli->pseudo[recu]='\0';
        printf("%s\n",cli->pseudo);
        int size = sprintf(buffer, "HELLO %s", cli->pseudo);

        send(fd,buffer,size,0);
        //todo: verifier diff -1
        memset(buffer,0,BUFF_SIZE);
    } else {
        printf(":((");
    }

    int run = 1;

    while (run) {
        memset(buffer,0,BUFF_SIZE);
        recu = recv(fd, buffer, (BUFF_SIZE-1)*sizeof(char),0);
        if (recu==0) {
            run=0;
        } else {



        memmove(tmp,buffer,3);
        if (recu>0) {

                tmp[recu]='\0';
                printf("%s",tmp);
                }
        uint16_t nb;

        if (strcmp(tmp, "INT")==0) {
            //printf("dog\n");
            pthread_mutex_lock(&lock);
            reception=1;
            memmove (&nb, buffer+3, 2);
            //todo: voir si marche ntohs
            nb = ntohs(nb);
            printf("%d\n",nb);
            if (nb > cli_max.nb) {
                cli_max.nb = nb;
            }
            char * rep = "INTOK";
            int nb_sent = send(fd, rep,strlen(rep)+1,0);
            if (nb_sent==-1) {
                perror("send");
                exit(1);
            }

            if (nb_sent>0) {
                        memset(tmp,0,BUFF_SIZE);
                        memset(buffer,0,BUFF_SIZE);

            } else {
                printf(":o\n");
            }
            //printf("nb send:%d\n",nb_sent);
            pthread_mutex_unlock(&lock);
        }

        else if (strcmp("MAX",buffer)==0) {
            pthread_mutex_lock(&lock);
            /*
            envoie au client la valeur maximale de
            l’entier parmi les entiers reçus.
            */
            if (reception) {
                memset(buffer,0,BUFF_SIZE);
                char * rep = "REP";
                //TODO: pb: faire un compteur mais quel type ? on a du 32 et du 16 bits
                int ctr=0;

                memmove(buffer,rep,strlen(rep));
                ctr += strlen(rep);
                // todo: ajouter caractère final a pseudo_max ???
                memmove(buffer+ctr,cli_max.pseudo,MAX_NAME);
                ctr += MAX_NAME;
                uint32_t ip = cli_max.addr->sin_addr.s_addr;
                ip = htonl(ip);

                memmove(buffer+strlen(rep)+MAX_NAME,&ip,sizeof(uint32_t));
                ctr += sizeof(uint32_t);
                uint16_t n = cli_max.nb;
                n =htons(n);
                memmove(buffer+strlen(rep)+MAX_NAME+sizeof(uint32_t), &n, sizeof(uint16_t));
                ctr += sizeof(uint16_t);
                int nb_sent = send (fd, buffer, BUFF_SIZE,0);
                if (nb_sent==-1) {
                    perror("send");
                    exit(1);
                }
                reception=1;
            } else {
                int nb_sent = send (fd, "NOP", 3,0);
                if (nb_sent==-1) {
                    perror("send");
                    exit(1);
                }
            }
            pthread_mutex_unlock(&lock);
        }
        else
                    {

                        sprintf(buffer, "UNKNOWN COMMAND: %s", buffer);
                        //send(fd, buffer, strlen(buffer),0);
                        memset(buffer,0,BUFF_SIZE);
                        run = 0;
                    }
                    }
    }
    free(tmp);
    // déconnexion du client
    close(fd);
    free(cli->pseudo);
    free(buffer);
    free(cli);
    return NULL;
}

