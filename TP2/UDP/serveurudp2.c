/**
 * serveur.c
 */

// On importe des fichiers.h n?c?ssaires ? l'application
#include <stdio.h>      // Fichier contenant les en-t?tes des fonctions standard d'entr?es/sorties 
#include <stdlib.h>     // Fichier contenant les en-t?tes de fonctions standard telles que malloc()
#include <string.h>     // Fichier contenant les en-t?tes de fonctions standard de gestion de cha?nes de caract?res 
#include <unistd.h>     // Fichier d'en-t?tes de fonctions de la norme POSIX (dont gestion des fichiers : write(), close(), ...)
#include <sys/types.h>      // Fichier d'en-t?tes contenant la d?finition de plusieurs types et de structures primitifs (syst?me)
#include <sys/socket.h>     // Fichier d'en-t?tes des fonctions de gestion de sockets
#include <netinet/in.h>     // Fichier contenant diff?rentes macros et constantes facilitant l'utilisation du protocole IP
#include <netdb.h>      // Fichier d'en-t?tes contenant la d?finition de fonctions et de structures permettant d'obtenir des informations sur le r?seau (gethostbyname(), struct hostent, ...)
#include <memory.h>     // Contient l'inclusion de string.h (s'il n'est pas d?j? inclus) et de features.h
#include <errno.h>      // Fichier d'en-t?tes pour la gestion des erreurs (notamment perror()) 

#define P 12332

/*
creersock

Il s'agit de la fonction qui va permettre la cr?ation d'une socket.
Elle est utilis?e dans la fonction main().
Elle prend en param?tre un entier court non sign?, qui est le num?ro de port,
n?c?ssaire ? l'op?ration bind().
Cette fonction renvoie un num?ro qui permet d'identifier la socket nouvellement cr??e
(ou la valeur -1 si l'op?ration a ?chou?e).
*/

int creersock( u_short port) {

  // On cr?e deux variables enti?res
  int sock, retour;

  // On cr?e une variable adresse selon la structure sockaddr_in (la structure est d?crite dans sys/socket.h)
  struct sockaddr_in adresse;
  
  /*
  La ligne suivante d?crit la cr?ation de la socket en tant que telle.
  La fonction socket prend 3 param?tres : 
  - la famille du socket : la plupart du temps, les d?veloppeurs utilisent AF_INET pour l'Internet (TCP/IP, adresses IP sur 4 octets)
    Il existe aussi la famille AF_UNIX, dans ce mode, on ne passe pas des num?ros de port mais des noms de fichiers.
  - le protocole de niveau 4 (couche transport) utilis? : SOCK_STREAM pour TCP, SOCK_DGRAM pour UDP, ou enfin SOCK_RAW pour g?n?rer
    des trames directement g?r?es par les couches inf?rieures.
  - un num?ro d?signant le protocole qui fournit le service d?sir?. Dans le cas de socket TCP/IP, on place toujours ce param?tre a 0 si on utilise le protocole par d?faut.
  */


  sock = socket(AF_INET,SOCK_DGRAM,0);

  // Si le code retourn? n'est pas un identifiant valide (la cr?ation s'est mal pass?e), on affiche un message sur la sortie d'erreur, et on renvoie -1
  if (sock<0) {
    perror ("ERREUR OUVERTURE");
    return(-1);
  }
  
  // On compl?te les champs de la structure sockaddr_in : 
  // La famille du socket, AF_INET, comme cit? pr?c?dement
  adresse.sin_family = AF_INET;

  /* Le port auquel va se lier la socket afin d'attendre les connexions clientes. La fonction htonl()  
  convertit  un  entier  long  "htons" signifie "host to network long" conversion depuis  l'ordre des bits de l'h?te vers celui du r?seau.
  */
  adresse.sin_port = htons(port);

  /* Ce champ d?signe l'adresse locale auquel un client pourra se connecter. Dans le cas d'une socket utilis?e 
  par un serveur, ce champ est initialis? avec la valeur INADDR_ANY. La constante INADDR_ANY utilis?e comme 
  adresse pour le serveur a pour valeur 0.0.0.0 ce qui correspond ? une ?coute sur toutes les interfaces locales disponibles.
    
  */
  adresse.sin_addr.s_addr=INADDR_ANY;
  
  /*
  bind est utilis? pour lier la socket : on va attacher la socket cr?e au d?but avec les informations rentr?es dans
  la structure sockaddr_in (donc une adresse et un num?ro de port).
   Ce bind affecte une identit? ? la socket, la socket repr?sent?e par le descripteur pass? en premier argument est associ?e 
    ? l'adresse pass?e en seconde position. Le dernier argument repr?sente la longueur de l'adresse. 
    Ce qui a pour but de  rendre la socket accessible de l'ext?rieur (par getsockbyaddr)
  */
  retour = bind (sock,(struct sockaddr *)&adresse,sizeof(adresse));
  
  // En cas d'erreur lors du bind, on affiche un message d'erreur et on renvoie -1
  if (retour<0) {
    perror ("IMPOSSIBLE DE NOMMER LA SOCKET");
    return(-1);
  }

  // Au final, on renvoie sock, qui contient l'identifiant ? la socket cr?e et attach?e.
  return (sock);
}


int main (int argc, char *argv[]) {

  // On d?finit les variables n?c?ssaires
  int sock;
  u_short port;
  char msg [BUFSIZ];

  if(argc != 2)
    port=P;
  else
    port = atoi(argv[1]);

  // On cr?e la socket
  sock = creersock (port);

  

  /*
  listen
	permet de dimensionner la taille de la file d'attente.
   On passe en param?tre la socket qui va ?couter, et un entier qui d?signe le nombre de connexions simultan?es autoris?es (backlog)
  */
  listen (sock,5);

  struct in_addr from = { 0 };
  socklen_t fromsize = sizeof from;
  int n = 0;
  while(1){
    /*
      Réception du message au sauvegarde des informations de l'émetteur.
    */
    if((n = recvfrom(sock, msg, sizeof msg - 1, 0, (struct sockaddr *) & from, &fromsize)) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }else{
        int f = fork();
        if(f == -1){
        perror("Erreur fork");
        exit(1);
        }
        if(f == 0){
            // On lit le message envoy? par la socket de communication. 
            //  msg contiendra la chaine de caract?res envoy?e par le r?seau,
            // s le code d'erreur de la fonction. -1 si pb et sinon c'est le nombre de caract?res lus
            if (n == -1)
                perror("Problemes");
            else {
                // Si le code d'erreur est bon, on affiche le message.
                msg[n] = 0;
                printf("Msg: %s\n", msg);
                printf("Recept reussie, emission msg: ");
                // On demande ? l'utilisateur de rentrer un message qui va ?tre exp?di? sur le r?seau
                scanf(" %[^\n]", msg);
                /*
                envoie du message grâce aux informations contenue dans le message qui a été reçut.
                */
                if(sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&from, fromsize) < 0)
                {
                    perror("sendto()");
                    exit(errno);
                }
                else
                    printf("Ecriture reussie, msg: %s\n", msg);
                // On referme la socket de communication
            }
            close(sock);
            exit(1);
        }
    }
  }

  // On referme la socket d'?coute.
  close(sock);
  
  return 0;
}