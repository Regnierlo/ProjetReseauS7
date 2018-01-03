all: serveurudp clientudp serveurudp2

serveurudp: serveurudp.o
	gcc -o serveurudp -lnsl  -Wall serveurudp.o
serveurudp.o: serveurudp.c
	gcc -c -Wall serveurudp.c

serveurudp2: serveurudp2.o
	gcc -o serveurudp2 -lnsl  -Wall serveurudp2.o
serveurudp2.o: serveurudp2.c
	gcc -c -Wall serveurudp2.c

clientudp: clientudp.o
	gcc -o clientudp -lnsl  -Wall clientudp.o
clientudp.o: clientudp.c
	gcc -c -Wall clientudp.c

