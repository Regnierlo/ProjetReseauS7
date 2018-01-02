/**
 * serveurmultiport.c
 */

// On importe des fichiers.h necessaires a l'application
#include <stdio.h>      // Fichier contenant les en-tetes des fonctions standard d'entr?es/sorties 
#include <stdlib.h>     // Fichier contenant les en-tetes de fonctions standard telles que malloc()
#include <string.h>     // Fichier contenant les en-tetes de fonctions standard de gestion de chaines de caracteres
#include <unistd.h>     // Fichier d'en-tetes de fonctions de la norme POSIX (dont gestion des fichiers : write(), close(), ...)
#include <sys/types.h>      // Fichier d'en-tetes contenant la d?finition de plusieurs types et de structures primitifs (systeme)
#include <sys/socket.h>     // Fichier d'en-tetes des fonctions de gestion de sockets
#include <netinet/in.h>     // Fichier contenant differentes macros et constantes facilitant l'utilisation du protocole IP
#include <netdb.h>      // Fichier d'en-tetes contenant la definition de fonctions et de structures permettant d'obtenir des informations sur le reseau (gethostbyname(), struct hostent, ...)
#include <memory.h>     // Contient l'inclusion de string.h (s'il n'est pas deja inclus) et de features.h
#include <errno.h>      // Fichier d'en-tetes pour la gestion des erreurs (notamment perror()) 

/*
creersockTCP

Il s'agit de la fonction qui va permettre la creation d'une socket.
Elle est utilisee dans la fonction main().
Elle prend en parametre un entier court non signe, qui est le numero de port,
necessaire a l'operation bind().
Cette fonction renvoie un numero qui permet d'identifier la socket nouvellement creee
(ou la valeur -1 si l'operation a echouee).
*/

int creersockTCP( u_short port) 
{

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

  sock = socket(AF_INET,SOCK_STREAM,0);

  // Si le code retourn? n'est pas un identifiant valide (la cr?ation s'est mal pass?e), on affiche un message sur la sortie d'erreur, et on renvoie -1
  if (sock<0)
  {
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
  if (retour<0)
  {
    perror ("IMPOSSIBLE DE NOMMER LA SOCKET");
    return(-1);
  }

  // Au final, on renvoie sock, qui contient l'identifiant ? la socket cr?e et attach?e.
  return (sock);
}

int creersockUDP( u_short port) 
{
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

int main (int argc, char *argv[])
{
  // On d?finit les variables n?c?ssaires
  int s, s1,s2;
  u_short port1;
  u_short port2;
  char msg [BUFSIZ];
  int next =0;
  int maxfd = 10;
  int newfd[10];
  struct in_addr from = { 0 };
  int fromsize = sizeof from;
 
  //on défini les ports ainsi : port1 pour le TCP et port2 pour l'UDP
  if(argc != 2)
  {
    port1=12332;
    port2=12333;
  }
  else
  {
    port1 = atoi(argv[1]);
    port2 = atoi(argv[1]);
  }

  // On cree les deux sockets, s1 pour les communications en TCP et s2 pour les communications en UDP
  s1=creersockTCP(port1);
  s2=creersockUDP(port2);

  /*
  listen permet de dimensionner la taille de la file d'attente.
   On passe en param?tre la socket qui va ?couter, et un entier qui d?signe le nombre de connexions simultan?es autoris?es (backlog)
  */
  listen(s1,5);
  listen(s2,5);

  while(1)
  {
    /* La fonction accept permet d'accepter une connexion ? notre socket par un client. On passe en param?tres la socket serveur d'�coute � demi d�finie.
    newsockfd contiendra l'identifiant de la socket de communication. newsockfd est la valeur de retour de la primitive accept. 
    C'est une socket d'?change de messages : elle est enti?rement d?finie.
    On peut pr?ciser aussi la structure et la taille de sockaddr associ?e 
    mais ce n'est pas obligatoire et ici on a mis le pointeur NULL
    */
    
    //cet appel permet de surveiller les descripteurs pour voir si des donnees en lectures sont disponibles, et evite qu'ils soient bloquants
    fd_set readfds;
    
    //on met à zéro le descripteur pour les données en lecture (sorte d'initialisation)
    FD_ZERO(&readfds);
    //on ajoute un descripteur pour relier la socket avec la lecture des données (on le fait pour la socket TCP et UDP)
    FD_SET(s1, &readfds);
    FD_SET(s2, &readfds);

    //on vérifie que le descripteur ne renvoit rien, sinon une erreur est affichée (tcp et udp)
    if(select(s1+1, &readfds,0,0,0)<0)
    {
        perror("Select s1");
        return(-1);
    }

    if(select(s2+1, &readfds,0,0,0)<0)
    {
        perror("Select s2");
        return(-1);
    }
    
    //on attend un changement sur la socket avec la lecture des données (si on reçoit un message)
    select(s1+1, &readfds, 0,0,0);
    select(s2+1, &readfds, 0,0,0);

    //dans cette boucle, on va rediriger selon si c'est du TCP ou UDP (normalement, et c'est là où ça bloque...) on arrive à lire le TCP avec le port 12332
    //mais pas l'UDP sur le port 12333
    for(next=0;next<=maxfd;next++)
    {
      //on vérifie que le descripteur s1 est contenu dans l'ensemble readfds, donc s'il y a une lecture de données possibles
      if(FD_ISSET(s1, &readfds))
      {
        //on accepte la connexion que l'on stock dans un tableau de socket
        newfd[next] = accept (s1,(struct sockaddr *) 0, (unsigned int*) 0);
      
        // Si l'accept se passe mal, on quitte le programme en affichant un message d'erreur.
        if (newfd[next] == -1) 
        {
          perror("Erreur accept\n");
          return(-1);
        }
        else
          printf("\nAccept reussi\n");

        int f = fork();
        if(f == -1)
        {
          perror("Erreur fork");
          exit(1);
        }
        if(f == 0)
        {
          close(s1);

          // On lit le message envoy? par la socket de communication. 
          //  msg contiendra la chaine de caract?res envoy?e par le r?seau,
          // s le code d'erreur de la fonction. -1 si pb et sinon c'est le nombre de caract?res lus
          s = read(newfd[next], msg, 1024);
          

          if (s == -1)
              perror("Problemes");
          else 
          {
            // Si le code d'erreur est bon, on affiche le message.
            msg[s] = 0;
            printf("Msg: %s\n", msg);
            printf("Recept reussie, emission msg: ");

            // On demande ? l'utilisateur de rentrer un message qui va ?tre exp?di? sur le r?seau
            scanf(" %[^\n]", msg);
              
            // On va ?crire sur la socket, en testant le code d'erreur de la fonction write.
            s = write(newfd[next], msg, strlen(msg));
            if (s == -1) 
            {
              perror("Erreur write");
              return(-1);
            }
            else
              printf("Ecriture reussie, msg: %s\n", msg);
              
            // On referme la socket de communication
            close(newfd[next]);
            exit(1);
          }
        }
      }
      if(FD_ISSET(s2, &readfds))
      {
        if((newfd[next]= recvfrom(s2, msg, sizeof msg - 1, 0, (struct sockaddr *) & from, &fromsize)) < 0)
        {
          perror("recvfrom() : ");
          exit(errno);
        }
        else
        {
          int f = fork();
          if(f == -1){
          perror("Erreur fork");
          exit(1);
          }
          if(f == 0){
              // On lit le message envoy? par la socket de communication. 
              //  msg contiendra la chaine de caract?res envoy?e par le r?seau,
              // s le code d'erreur de la fonction. -1 si pb et sinon c'est le nombre de caract?res lus
              if (msg == -1)
                  perror("Problemes");
              else {
                  // Si le code d'erreur est bon, on affiche le message.
                  msg[newfd[next]] = 0;
                  printf("Msg: %s\n", msg);
                  printf("Recept reussie, emission msg: ");
                  // On demande ? l'utilisateur de rentrer un message qui va ?tre exp?di? sur le r?seau
                  scanf(" %[^\n]", msg);
                  /*
                  envoie du message grâce aux informations contenue dans le message qui a été reçut.
                  */
                  if(sendto(s2, msg, strlen(msg), 0, (struct sockaddr *)&from, fromsize) < 0)
                  {
                      perror("sendto()");
                      exit(errno);
                  }
                  else
                      printf("Ecriture reussie, msg: %s\n", msg);
                  // On referme la socket de communication
              }
              close(s2);
              close(newfd[next]);
              exit(1);
          }
        }
      }
    }    
  }
  // On referme la socket d'?coute.
  close(s1);
  close(s2);
  return 0;
}
