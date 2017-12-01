all: serveurtcp clienttcp

serveurtcp: serveurtcp.o
	gcc -o serveurtcp -lnsl  -Wall serveurtcp.o
serveurtcp.o: serveurtcp.c
	gcc -c -Wall serveurtcp.c

clienttcp: clienttcp.o
	gcc -o clienttcp -lnsl  -Wall clienttcp.o
clienttcp.o: clienttcp.c
	gcc -c -Wall clienttcp.c

