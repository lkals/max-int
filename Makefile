all: serveur client1 client2

serveur : serveur.c
	gcc -pthread -Wall -c serveur.c
	gcc -o serveur serveur.o
client1 : client1.c
	gcc -pthread -Wall -c client1.c
	gcc -o client1 client1.o
client2 : client2.c
	gcc -pthread -Wall -c client2.c
	gcc -o client2 client2.o
distclean :
	rm *o