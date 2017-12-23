#include <stdio.h>          // Fichier contenant les en-tetes des fonctions standard d'entr�es/sorties 
#include <stdlib.h>         // Fichier contenant les en-tetes de fonctions standard telles que malloc()
#include <string.h>         // Fichier contenant les en-tetes de fonctions standard de gestion de chaines de caracteres 
#include <unistd.h>         // Fichier d'en-tetes de fonctions de la norme POSIX (dont gestion des fichiers : write(), close(), ...)
#include <sys/types.h>      // Fichier d'en-tetes contenant la definition de plusieurs types et de structures primitifs (syst�me)
#include <sys/socket.h>     // Fichier d'en-tetes des fonctions de gestion de sockets
#include <netinet/in.h>     // Fichier contenant differentes macros et constantes facilitant l'utilisation du protocole IP
#include <netdb.h>          // Fichier d'en-tetes contenant la definition de fonctions et de structures permettant d'obtenir des informations sur le r�seau (gethostbyname(), struct hostent, ...)
#include <memory.h>         // Contient l'inclusion de string.h (s'il n'est pas deja inclus) et de features.h
#include <errno.h>          // Fichier d'en-tetes pour la gestion des erreurs (notamment perror()) 


#include "clientftp.h"
#include "fonctionscmd.h"
#include "struct.h"

//Port utilise par defaut
#define Port 21
//A la lecture du code permet de mieux comprendre quand il y a erreur
#define ERREUR -1
//Definition d une taille maximale
#define MAXSIZE BUFSIZ

int main(int argc, char *argv[])
{
    //VERIFICATION LANCEMENTE
    /*
    *    verification du nombre d'argument
    *    1 attendu => adresse
    *    (2 avec le argv[0] qui est le nom du fichier)
    */
    if (argc < 2)
    {
        perror("Adresse manquante");
        exit(errno);
    }   
    
    //-----------------------------------------------------------------------

    //DEFINITION DES VARIABLES
    //Pour la connection et plus
    char* ip = (char*)malloc(sizeof(char)*MAXSIZE);
    int nb_byte; //nombre d octets envoye ou recu
    int verifLecture;
    int sock; 
    char msgSrv[BUFSIZ];//pour les lignes entrees par l utilisateur
    char login[BUFSIZ];//login utilisateur
    char logintmp[BUFSIZ];
    char *mdp;//mdp utilisateur
    char mdptmp[BUFSIZ];
    char cmd[MAXSIZE];
    char cmdtmp[MAXSIZE];
    int numCmd;
    int pasv;
    int envoieMessage;
    int quitter;

    //Pour la gestion des fichiers
    char *dossierCourant = (char*)malloc(sizeof(char)*MAXSIZE);
    char *dossierCourantLocal = (char*)malloc(sizeof(char)*MAXSIZE);
    struct listeArgument listeArgumentsCommande;
    

    struct sockaddr_in adresse; //pour la socket cliente
    struct hostent *recup; //recherche adresse IP pour le serveur

    //Initialisation
    //pour les retours du serveur
    int i = 0;
    for(;i<BUFSIZ;i++)
        msgSrv[i] = '.';

    //indication du mode passif ou non (boolean)
    pasv = 0;
    //indique si l envoie du message est neccessaire
    envoieMessage = 0;
    //savoir si le client faut arreter le logiciel
    quitter = 0;
    //recuperation ip
    strcpy(ip,argv[1]);

    //-------------------------------------------------------------------------
    
    //CREATION SOCKET
    sock = socket(AF_INET,SOCK_STREAM,0); //Declaration ipv4 + tcp
    if(sock <= ERREUR)
    {
        perror("Erreur ouverture");
        exit(errno);
    }

    recup = gethostbyname(ip);//Resolution adresse serveur
    if(recup == NULL)
    {
        perror("Erreur obtention adresse");
        exit(errno);
    }

    //Affichage avance
    printf("Resolution adresse\n");

    //Integration de l adresse serveur dans la socket cliente
    memcpy((char *)&adresse.sin_addr, (char *)recup->h_addr, recup->h_length);
    adresse.sin_family = AF_INET;
    adresse.sin_port = htons((u_short) Port);//Ajout du numero du port

    //Connection
    if(connect(sock,(struct sockaddr *)&adresse,sizeof(adresse)) == ERREUR)
    {
        perror("Erreur connexion");
        exit(errno);
    }

    //Affichage avance
    printf("Connection établie à %s:%d\n",ip,(int)Port);

    //-------------------------------------------------------------------------
    verifLecture = read(sock,msgSrv,MAXSIZE);
    if(verifLecture == ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    //Lecture de la banniere d acceuil
    afficheReponse(verifLecture,msgSrv);

    //-------------------------------------------------------------------------
    //CONNECTION UTILISATEUR
    //envoie login
    printf("Login : ");
    scanf(" %[^\n]",login);
    sprintf(logintmp,"USER %s\r\n",login); //creation d une chaine plus complete a envoyer au serveur
    
    if(write(sock,logintmp,strlen(logintmp)) == ERREUR)
    {
        perror("Erreur connection");
        exit(errno);
    }

    //reponse serveur
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
    
    //envoie mdp
    mdp = (char *)malloc(4096); //necessaire pour recuperer le mot de passe ligne suivante
    mdp = getpass("Mot de passe : "); //recuperation du mot de passe cache
    sprintf(mdptmp,"PASS %s\r\n",mdp);
    free(mdp);//liberation de la memoire allouee

    if(write(sock,mdptmp,strlen(mdptmp)) == ERREUR)
    {
        perror("Erreur connection");
        exit(errno);
    }

    //reponse serveur
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    //info serveur
    sprintf(cmd,"SYST\r\n");
    if(write(sock,cmd,strlen(cmd)) == ERREUR)
    {
        perror("Erreur SYST");
        exit(errno);
    }

    verifLecture = read(sock, msgSrv, strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    //-------------------------------------------------------------------------
    //TRAITEMENT FICHIERS
    
    //Recuperation dossier courant
    strcpy(cmd,"PWD\r\n");
    
    if(write(sock,cmd,strlen(cmd)) == ERREUR)
    {
        perror("Erreur connection");
        exit(errno);
    }

    //reponse serveur
    verifLecture = read(sock, msgSrv, strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    //Sauvegarde des dossiers courants
    //sauvegarde du dossier distant
    recuperationDossierCourant(verifLecture,msgSrv,dossierCourant);
    //recuperation et sauvegarde du dossier local
    getcwd(dossierCourantLocal,MAXSIZE);


    do
    {
        printf("ftp> ");
        fflush(stdout);
        scanf(" %[^\n]",cmdtmp);

        /***************** RECUPERATION COMMANDE *****************/
        //recuperation de la commande
        recuperationCommande(cmdtmp,cmd);
        //recuperation des arguments
        listeArgumentsCommande = recuperationArguments(cmdtmp);

        //recuperation num commande
        numCmd = getnumcmd(cmdtmp);


        switch(numCmd)
        {
            case CD:
                strcpy(cmd,cmd_cd(cmdtmp,listeArgumentsCommande));
                envoieMessage = 1;
                break;
            case LCD:
                 cmd_lcd(listeArgumentsCommande.arguments[1]);
                envoieMessage = 0;
                break;
            case PWD:
                strcpy(cmdtmp,"PWD");
                envoieMessage = 1;
                break;
            case LPWD:
                cmd_lpwd();
                envoieMessage = 0;
                break;
            case LS:
                cmd_ls(ip,cmdtmp,sock);
                envoieMessage = 0;
                break;
            case LLS:
                cmd_lls(listeArgumentsCommande);
                envoieMessage = 0;
                break;
            case QUIT:
                strcpy(cmdtmp,"QUIT");
                quitter = 1;
                envoieMessage = 1;
                break;
            case GET:
                cmd_get(ip,listeArgumentsCommande, sock);
                envoieMessage = 0;
                break;
            case PUT:
                printf("Non implémentée\n");
                envoieMessage = 0;
                break;
            case RM:
                printf("Non implémentée\n");
                envoieMessage = 0;
                break;
            case LRM:
                printf("Non implémentée\n");
                envoieMessage = 0;
                break;
            case MKDIR:
                printf("Non implémentée\n");
                envoieMessage = 0;
                break;
            case LMKDIR:
                printf("Non implémentée\n");
                envoieMessage = 0;
                break;
            default:
                printf("COMMANDE INCONNUE\n");
                printf("NUM COMMANDE : %d\n",numCmd);
                break;
        }
        

        
        if(envoieMessage == 1)//traitement a distance
        {
            sprintf(cmd,"%s\r\n",cmdtmp);
            
            //envoie au serveur
            if(write(sock,cmd,strlen(cmd)) == ERREUR)
            {
                perror("Erreur connection");
                exit(errno);
            }
            
            //reponse serveur
            verifLecture = read(sock,msgSrv,strlen(msgSrv));
            if(verifLecture <= ERREUR)
            {
                perror("Erreur read");
                exit(errno);
            }
            afficheReponse(verifLecture,msgSrv);
        }

        //liberation memoire de la liste des arguments
        liberedelivre(listeArgumentsCommande);

    }while(quitter != 1);
    //-------------------------------------------------------------------------
    //Fermeture de la socket
    close(sock);

    //liberation memoire
    free(ip);
    free(dossierCourant);
    free(dossierCourantLocal);
    
    return 0;
}




//liberation de la memoire pour les arguments
void liberedelivre(struct listeArgument la)
{
    int i=0;
    
    for(;i<la.nbarg;i++)
        free(la.arguments[i]);

    free(la.arguments);
}

