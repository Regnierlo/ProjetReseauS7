CC=gcc
EXEC=clientftp

all: $(EXEC)

clientftp: clientftp.o fonctionscmd.o
	$(CC) -o gcc -o clientftp -lnsl  -Wall clientftp.o fonctionscmd.o
clientudp.o: clientftp.c clientftp.h
	$(CC) -c -Wall clientftp.c
fonctionscmd.o: fonctionscmd.c fonctionscmd.h
	$(CC) -c -Wall fonctionscmd.c