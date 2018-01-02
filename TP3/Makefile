all: clienttcp clientudp serveurmultiport

clienttcp: clienttcp.o
	gcc -o clienttcp -lnsl  -Wall clienttcp.o
clienttcp.o: clienttcp.c
	gcc -c -Wall clienttcp.c

clientudp: clientudp.o
	gcc -o clientudp -lnsl  -Wall clientudp.o
clientudp.o: clientudp.c
	gcc -c -Wall clientudp.c

serveurmultiport: serveurmultiport.o
	gcc -o serveurmultiport -lnsl  -Wall serveurmultiport.o
serveurmultiport.o: serveurmultiport.c
	gcc -c -Wall serveurmultiport.c
