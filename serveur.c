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

/*
Le protocole maxint côté serveur
Après avoir salué le client avec le message « HELLO␣<pseudo> », le serveur répond aux requêtes
du client de la façon suivante :
– s’il reçoit un message de type « INT ». Il renvoie le message « INTOK » (Message OK) au client pour dire qu’il a reçu son message,
– s’il reçoit un message de type « MAX », il envoie au client la valeur maximale de l’entier parmi les entiers reçus. Il envoie donc « REP<pseudo><ip><val> » où <pseudo> est le nom de l’utilisateur ayant envoyé le plus grand entier, <ip> est son adresse ip codée sur 4 octets en big-endian, et <val> est la valeur du plus grand entier reçu par le serveur codé en big-endian. Il n’y a pas d’espace entre le pseudo, l’adresse ip et les données. Si le serveur n’a pas encore reçu d’entier, il répond par « NOP ». Si plusieurs clients ont envoyé la valeur la plus grande, le serveur renverra l’identité du dernier client ayant envoyé cette valeur.
Attention, veillez à bien gérer les accès concurrents pour les valeurs sauvegardées par le serveur.
Notes : le champ sin_addr.s_addr de la structure struct sockaddr_in est un entier codé sur 4 octets en écriture big-endian.
Pour afficher un entier sous forme hexadécimal avec la fonction printf, utilisez la spécification de format %x.
*/

typedef struct {
    int * fd;
    struct sockaddr_in * addr;
    //char * pseudo;
    // ajout d'informations utiles et pour compater le code
    char pseudo[10];
    uint16_t nb;
} cli_info;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// todo: ça devrait ê un pointeur ?
cli_info cli_max;
// booléen indiquant si on a reçu un entier ou pas
int reception;

// TODO: tester sur lulu et sur d'autres ordis
// TODO: faire un goto error si temps

//todo: utiliser les fonctions mem (memmove plutot que mempcpy) pour envoyer des suites d'octets (envoyer des int en big endian et non pas des string

void * maxint(void *arg);

int main(int argc, char *argv[]) {

    //todo: vérifier qu'un numéro de port est en argument
    if (argc!=2) {
        fprintf(stderr,"nombre d'arguments : ajouter numéro de port aux arguments du programme \n");
        exit(1);
    }
    //todo: traiter argv[1]

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd==-1) {
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
    address_sock.sin_addr.s_addr=htonl(INADDR_ANY);

    int r = bind(sockfd, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
    if (r==-1) {
        perror("bind");
        exit(1);
    }
    if (r==0) {
        r = listen(sockfd,0);
        if (r==0) {
            struct sockaddr_in caller;
            socklen_t size = sizeof(caller);
            int * sockcli = (int *)malloc(sizeof(int));
            cli_info * cli = malloc (sizeof(cli_info));
            while (1) {
                *sockcli= accept(sockfd, (struct sockaddr *)&caller, &size);
                cli->fd = sockcli;
                cli->addr = &caller;
                pthread_t th;
                pthread_create (&th, NULL, maxint,cli);
            }
        }
        // todo: faire les free et close nécessaires
    }
    return 0;
}

void * maxint(void *arg) {

    cli_info * cli = ((cli_info *)arg);
    int fd = *(cli->fd);

    // on attend le message du client <pseudo>
    int recu = recv(fd, cli->pseudo, 10*sizeof(char),0);
    if (recu==0) {
        close(fd);
    }
    //todo: attention au charactère final
    cli->pseudo[recu]='\0';

    char * mess = malloc(100);
    char * hello ="HELLO";
    memmove(mess,hello,strlen(hello));
    memmove(mess+strlen(hello),cli->pseudo, MAX_NAME);
    int nb_send = send(fd,mess,strlen(mess)+1,0);
    free(mess);

    if (nb_send != -1) {
        pthread_mutex_lock(&lock);

        char * buff=malloc(100);
        int recu = recv(fd, buff, 99*sizeof(char),0);
        if (recu==0) {
            close(fd);
        }

        char * tmp = malloc(100);
        memmove(tmp, buff, 3);
        uint16_t nb;
        if (strcmp(tmp, "INT")==0) {
            memmove (&nb, buff+3, 2);
            //todo: voir si marche ntohs
            nb = ntohs(nb);
            if (nb > cli_max.nb) {
                cli_max.nb = nb;
            }
            char * rep = "INTOK";
            int nb_sent = send(fd, rep,strlen(rep)+1,0);
            if (nb_sent==-1) {
                perror("send");
                exit(1);
            }
        }
        free(tmp);
        if (strcmp("MAX",buff)==0) {
            /*
            envoie au client la valeur maximale de
            l’entier parmi les entiers reçus.
            */
            if (reception) {
                char * mess = malloc(512);
                char * rep = "REP";
                //TODO: pb: faire un compteur mais quel type ? on a du 32 et du 16 bits
                uint32_t ctr;

                memmove(mess,rep,strlen(rep));
                // todo: ajouter caractère final a pseudo_max ???
                memmove(mess+strlen(rep),cli_max.pseudo,MAX_NAME);
                uint32_t ip = cli_max.addr->sin_addr.s_addr;
                ip = htonl(ip);
                memmove(mess+strlen(rep)+MAX_NAME,&ip,sizeof(uint32_t));
                uint16_t n = cli_max.nb;
                n =htons(n);
                memmove(mess+strlen(rep)+MAX_NAME+sizeof(uint32_t), &n, sizeof(uint16_t));
                int nb_sent = send (fd, mess, 512,0);
                if (nb_sent==-1) {
                    perror("send");
                    exit(1);
                }
                free(mess);
                reception=1;
            } else {
                char * rep = "NOP";
                int nb_sent = send (fd, rep, strlen(rep)+1,0);
                if (nb_sent==-1) {
                    perror("send");
                    exit(1);
                }
            }
        }
        free(buff);
        pthread_mutex_unlock(&lock);
    }
    // déconnexion du client
    close(fd);
    return NULL;
}

