all: serveurudp

serveurudp: serveurudp.o
	gcc -o serveurudp -lnsl  -Wall serveurudp.o
serveurudp.o: serveurudp.c
	gcc -c -Wall serveurudp.c

