/**
 * client.c
 */

// On importe les fichiers.h ncéssaires.
#include <stdio.h>      // Fichier contenant les en-têtes des fonctions standard d'entrées/sorties 
#include <stdlib.h>     // Fichier contenant les en-têtes de fonctions standard telles que malloc()
#include <string.h>     // Fichier contenant les en-têtes de fonctions standard de gestion de chaînes de caractères 
#include <unistd.h>     // Fichier d'en-têtes de fonctions de la norme POSIX (dont gestion des fichiers : write(), close(), ...)
#include <sys/types.h>      // Fichier d'en-têtes contenant la définition de plusieurs types et de structures primitifs (système)
#include <sys/socket.h>     // Fichier d'en-têtes des fonctions de gestion de sockets
#include <netinet/in.h>     // Fichier contenant différentes macros et constantes facilitant l'utilisation du protocole IP
#include <netdb.h>      // Fichier d'en-têtes contenant la définition de fonctions et de structures permettant d'obtenir des informations sur le réseau (gethostbyname(), struct hostent, ...)
#include <memory.h>     // Contient l'inclusion de string.h (s'il n'est pas déjà inclus) et de features.h
#include <errno.h>      // Fichier d'en-têtes pour la gestion des erreurs (notamment perror()) 


#define P 12332
#define LGMES 200

int main() {

    // On définit les variables utilisées dans le programme client.
    int s, sock;
    char msg[BUFSIZ], NomDistant[BUFSIZ];

    // On crée une variable de type sokaddr_in qui sera utilisée pour la création de la socket cliente.
    struct sockaddr_in adresse;

    // Ce pointeur sur la structure hostent sera utilisé pour la recherche d'une adresse IP connaissant le nom du serveur (résolution de noms)
    struct hostent *recup;
    
    // On crée la socket cliente, de type AF_INET, qui utilisera TCP comme protocole de transport.
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur ouverture");
        return(-1);
    }
    
    // On demande a l'utilisateur de rentrer le nom de la machine serveur a laquelle le programme va se connecter.
    printf("Nom de la machine distante: ");
    scanf("%s", NomDistant);
    printf("NomDistant: %s", NomDistant);
    
    // On tente de résoudre le nom donné par l'utilisateur, afin de récupérer l'adresse qui lui correspond
// et remplir ainsi la structure sockaddr_in
    recup = gethostbyname(NomDistant);
    if (recup == NULL) {
        perror("Erreur obtention adresse");
        return(-1);
    }

    /*
    On copie la zone mémoire qui contient la valeur de reecup (l'adresse IP du serveur) directement dans la zone mémoire 
    qui doit contenir cette valeur dans la structure sockaddr_in. 
    En paramètres :  la zone mémoire destination, la zone mémoire source,
    la longueur des données a copier.
    */
    memcpy((char *)&adresse.sin_addr, (char *)recup->h_addr, recup->h_length);
    
    // On utilise toujours une socket de type AF_INET
    adresse.sin_family = AF_INET;

    // On place le numéro de port
    adresse.sin_port = htons((u_short) P);

    // On tente de se connecter au serveur : cette opération prend la place du accept que l'on a effectué dans le programme serveur.
    if (connect(sock, (struct sockaddr *)&adresse, sizeof(adresse)) == -1) {
        perror("Erreur connexion");
        return(-1);
    }
    
    /*
    On demande à l'utilisateur d'écrire un message qui sera envoyé via le réseau au serveur.
    On remarque que, contrairement au serveur qui lit en premier et écrit ensuite, le client commence par écrire le message
    que le serveur attend, et lit ensuite la réponse.
    */

    printf("\nMessage: ");
    scanf("%s", msg);
   // printf("Msg a envoyer: %s\n", msg);

    // Comme pour le serveur, on utilise write pour expédier le message sur le réseau, avec message d'erreur si l'écriture se passe mal.
    if (write(sock, msg, strlen(msg)) == -1) {
        perror("Erreur ecriture");
        return(-1);
    }
    
    // On lit la réponse du serveur.
    printf("Maintenant lecture\n");
    memset(msg, 0, sizeof msg);
    s = read(sock, msg, 1024);

    // Message d'erreur si la lecture se passe mal.
    if (s == -1) {
        perror("Erreur read");
        return(-1);
    }
    else
        printf("Lecture reussie, msg: %d %s\n", s, msg);
    
    // On referme la socket cliente. Cette opération ferme la socket cliente pour le programme client, le serveur fait de même de son coté,
    // en plus de refermer sa propre socket.
    close(sock);
    
    return 0;
}
