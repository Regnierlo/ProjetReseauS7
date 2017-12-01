all: serveurudp clientudp

serveurudp: serveurudp.o
	gcc -o serveurudp -lnsl  -Wall serveurudp.o
serveurudp.o: serveurudp.c
	gcc -c -Wall serveurudp.c

clientudp: clientudp.o
	gcc -o clientudp -lnsl  -Wall clientudp.o
clientudp.o: clientudp.c
	gcc -c -Wall clientudp.c

