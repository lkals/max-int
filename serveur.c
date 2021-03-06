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

//todo: code trop long dans les fonctions → utiliser des fonctions
//todo: limiter les mutex aux portions de code critique

typedef struct {
    uint16_t nb;
    uint32_t ip;
    char *pseudo;
} max_int;

typedef struct {
    max_int *max_item;
    int fd;
    struct sockaddr_in * addr;
} cli_info;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// booléen indiquant si on a reçu un entier ou pas
int reception=0;


void * maxint(void *arg);
void * handle_int_response(void *arg, pthread_mutex_t lock);
void * handle_max_response(void *arg, pthread_mutex_t lock);

int main(int argc, char *argv[]) {
    max_int* max = malloc(sizeof(max_int));
    if (max==NULL) {
        perror("malloc");
        exit(1);
    }
    max->pseudo=malloc(MAX_NAME+1);
    max->nb=0;
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
    pthread_t th;
    if (r==0) {
        r = listen(sockfd,0);
            while (1) {
                struct sockaddr_in caller;
                socklen_t size = sizeof(caller);
                int * sockcli = (int *)malloc(sizeof(int));
                cli = malloc (sizeof(cli_info));
                if (cli==NULL) {
                        perror("malloc");
                        exit(1);
                    }
                *sockcli = accept(sockfd, (struct sockaddr *)&caller, &size);
                cli->max_item = max;
                cli->fd = *sockcli;
                cli->addr = &caller;

                if (*sockcli>=0) {
                    pthread_create (&th, NULL, maxint,cli);
                }
                free(sockcli);
            }
            free(max->pseudo);
            free(max);
            close(sockfd);
            return 0;
    } else {
        close(sockfd);
        free(max->pseudo);
        free(max);
        perror("Can't start server");
        return 1;
    }
}

void * maxint(void *arg) {

    cli_info * cli = (cli_info *)arg;
    if (cli->max_item->pseudo==NULL) {
       printf("NULL \n");
    }
    if (reception !=0 && cli->max_item->pseudo!=NULL && strcmp("",cli->max_item->pseudo)==0) {
        printf("err : empty\n");
        close(cli->fd);
        free(cli);
        return NULL;
    }
    char * name = calloc (MAX_NAME+1,sizeof(char));
    char * buffer = calloc(BUFF_SIZE,sizeof(char));
    char * tmp = calloc(BUFF_SIZE,sizeof(char));

    int fd = (cli->fd);

    // on attend le message du client <pseudo>
    int recu = recv(cli->fd, name, MAX_NAME,0);
    if (recu==-1) {
        perror("recv");
        exit(1);
    }
    if (recu==0) {
        fprintf(stderr, "%s", "err: empty\n");
        close(fd);
        return NULL;
    }
    name[recu]='\0';
    printf("%s\n",name);
    int size = sprintf(buffer, "HELLO %s", name);
    send(fd,buffer,size,0);

    int run = 1;
    while (run) {
        memset(tmp,0,BUFF_SIZE);
        memset(buffer,0,BUFF_SIZE);
        recu = recv(fd, buffer, (BUFF_SIZE-1),0);
        if (recu==-1) {
            perror("recv");
            run=0;
        }
        if (recu==0) {
            run=0;
        } else {

        memmove(tmp,buffer,3);

        uint16_t nb;

        if (strcmp(tmp, "INT")==0) {
            memset(tmp,0,BUFF_SIZE);
            memmove(tmp,buffer,4);
            tmp[recu]='\0';
            printf("%s",tmp);
            char * rep = "INTOK";
            int nb_sent = send(fd, rep,strlen(rep),0);
            if (nb_sent==-1) {
                perror("send");
                run=0;
            }
            pthread_mutex_lock(&lock);
            reception=1;
            memmove (&nb, buffer+4, 2);
            nb = ntohs(nb);
            printf("%d\n",nb);
            if (nb >= cli->max_item->nb) {
                printf("nb:%d >= max_nb:%d\n",nb,cli->max_item->nb);
                cli->max_item->nb = nb;
                cli->max_item->ip = cli->addr->sin_addr.s_addr;
                strcpy(cli->max_item->pseudo,name);
            }
            pthread_mutex_unlock(&lock);
        }
        else if (strcmp("MAX",buffer)==0) {
            tmp[recu]='\0';
            printf("%s\n",tmp);

            pthread_mutex_lock(&lock);
            /*
            envoie au client la valeur maximale de
            l’entier parmi les entiers reçus.
            */
            if (reception) {
                memset(buffer,0,BUFF_SIZE);
                char * rep = "REP";
                int buff_size_ctr=0;

                memmove(buffer,rep,strlen(rep));
                buff_size_ctr += strlen(rep);
                memmove(buffer+buff_size_ctr,cli->max_item->pseudo,MAX_NAME);
                buff_size_ctr += MAX_NAME;
                uint32_t ip = cli->max_item->ip;
                ip = htonl(ip);

                memmove(buffer+buff_size_ctr,&ip,sizeof(uint32_t));
                buff_size_ctr += sizeof(uint32_t);
                uint16_t n = cli->max_item->nb;
                n =htons(n);

                memmove(buffer+buff_size_ctr, &n, sizeof(uint16_t));
                buff_size_ctr += sizeof(uint16_t);
                int nb_sent = send (fd, buffer, buff_size_ctr,0);
                if (nb_sent==-1) {
                    perror("send");
                    run=0;
                }
                reception=1;
            } else {
                int nb_sent = send (fd, "NOP", 3,0);
                if (nb_sent==-1) {
                    perror("send");
                    run=0;
                }
            }
            pthread_mutex_unlock(&lock);
        } else {

            run = 0;
        }
        }
    }
    free(tmp);
    printf("Connexion closed\n");
    // déconnexion du client
    close(cli->fd);
    free(buffer);
    free(name);
    free(cli);
    return NULL;
}

