/**
 * client.c
 */

// On importe les fichiers.h nc�ssaires.
#include <stdio.h>      // Fichier contenant les en-t�tes des fonctions standard d'entr�es/sorties 
#include <stdlib.h>     // Fichier contenant les en-t�tes de fonctions standard telles que malloc()
#include <string.h>     // Fichier contenant les en-t�tes de fonctions standard de gestion de cha�nes de caract�res 
#include <unistd.h>     // Fichier d'en-t�tes de fonctions de la norme POSIX (dont gestion des fichiers : write(), close(), ...)
#include <sys/types.h>      // Fichier d'en-t�tes contenant la d�finition de plusieurs types et de structures primitifs (syst�me)
#include <sys/socket.h>     // Fichier d'en-t�tes des fonctions de gestion de sockets
#include <netinet/in.h>     // Fichier contenant diff�rentes macros et constantes facilitant l'utilisation du protocole IP
#include <netdb.h>      // Fichier d'en-t�tes contenant la d�finition de fonctions et de structures permettant d'obtenir des informations sur le r�seau (gethostbyname(), struct hostent, ...)
#include <memory.h>     // Contient l'inclusion de string.h (s'il n'est pas d�j� inclus) et de features.h
#include <errno.h>      // Fichier d'en-t�tes pour la gestion des erreurs (notamment perror()) 


#define Port 12332
#define LGMES 200

int main(int argc, char *argv[]) {

    u_short port;

    if(argc != 2)
        port=Port;
    else
        port = atoi(argv[1]);

    // On d�finit les variables utilis�es dans le programme client.
    int sock;
    char msg[BUFSIZ], NomDistant[BUFSIZ];

    // On cr�e une variable de type sokaddr_in qui sera utilis�e pour la cr�ation de la socket cliente.
    struct sockaddr_in adresse;

    // Ce pointeur sur la structure hostent sera utilis� pour la recherche d'une adresse IP connaissant le nom du serveur (r�solution de noms)
    struct hostent *recup;
    
    // On cr�e la socket cliente, de type AF_INET, qui utilisera TCP comme protocole de transport.
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur ouverture");
        return(-1);
    }
    
    // On demande a l'utilisateur de rentrer le nom de la machine serveur a laquelle le programme va se connecter.
    printf("Nom de la machine distante: ");
    scanf("%s", NomDistant);
    printf("NomDistant: %s", NomDistant);
    
    // On tente de r�soudre le nom donn� par l'utilisateur, afin de r�cup�rer l'adresse qui lui correspond
// et remplir ainsi la structure sockaddr_in
    recup = gethostbyname(NomDistant);
    if (recup == NULL) {
        perror("Erreur obtention adresse");
        return(-1);
    }

    /*
    On copie la zone m�moire qui contient la valeur de reecup (l'adresse IP du serveur) directement dans la zone m�moire 
    qui doit contenir cette valeur dans la structure sockaddr_in. 
    En param�tres :  la zone m�moire destination, la zone m�moire source,
    la longueur des donn�es a copier.
    */
    memcpy((char *)&adresse.sin_addr, (char *)recup->h_addr, recup->h_length);
    
    // On utilise toujours une socket de type AF_INET
    adresse.sin_family = AF_INET;

    // On place le num�ro de port
    adresse.sin_port = htons((u_short) port);

    // On tente de se connecter au serveur : cette op�ration prend la place du accept que l'on a effectu� dans le programme serveur.
    if (connect(sock, (struct sockaddr *)&adresse, sizeof(adresse)) == -1) {
        perror("Erreur connexion");
        return(-1);
    }
    
    /*
    On demande � l'utilisateur d'�crire un message qui sera envoy� via le r�seau au serveur.
    On remarque que, contrairement au serveur qui lit en premier et �crit ensuite, le client commence par �crire le message
    que le serveur attend, et lit ensuite la r�ponse.
    */

    printf("\nMessage: ");
    scanf(" %[^\n]",msg);
    printf("Msg a envoyer: %s\n", msg);
    
    if(sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)0, 0) < 0)
    {
        perror("sendto()");
        exit(errno);
    }
    
    // Comme pour le serveur, on utilise write pour exp�dier le message sur le r�seau, avec message d'erreur si l'�criture se passe mal.
    /*if (write(sock, msg, strlen(msg)) == -1) {
        perror("Erreur ecriture");
        return(-1);
    }*/
    
    // On referme la socket cliente. Cette op�ration ferme la socket cliente pour le programme client, le serveur fait de m�me de son cot�,
    // en plus de refermer sa propre socket.
    close(sock);
    
    return 0;
}
