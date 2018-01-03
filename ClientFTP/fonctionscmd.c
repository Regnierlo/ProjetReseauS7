#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h> //pour manipuler les fichiers
#include <sys/stat.h> //pour recuperer info fichiers
#include "struct.h"

#define ERREUR -1

//Affiche la réponse du serveur
void afficheReponse(int length, char *msgSrv)
{
    msgSrv[length] = 0;
    printf("%s",msgSrv);
    msgSrv[length] = '.';
    fflush(stdout);
}

//calcul du port pour la connexion en mode passif
int getportpasv(int size, char* msg)
{
    int champAdr = 0;
    int p1 = 0;
    char* cp1 = (char*)malloc(5*sizeof(char));
    int p2 = 0;
    char* cp2 = (char*)malloc(5*sizeof(char));

    //recuperation des deux informations pour recalculer le port
    int i=0;
    for(;i<=size;i++)
    {
        if(msg[i] == '(') //1er champ de l adr ip
            champAdr = 1;
        else if(champAdr == 1 && msg[i] == ',')//2eme champ de l adr ip
            champAdr = 2;
        else if(champAdr == 2 && msg[i] == ',')//3eme champ de l adr ip
            champAdr = 3;
        else if(champAdr == 3 && msg[i] == ',')//4eme champ de l adr ip
            champAdr = 4;
        else if(champAdr == 4 && msg[i] == ',')//5eme champ de l adr ip - 1ere info utile
            champAdr = 5;
        else if(champAdr == 5 && msg[i] != ',')//recuperation de l info
        {
            cp1[p1] = msg[i];
            p1++;
        }
        else if(champAdr == 5 && msg[i] == ',')//6eme champ de l adr ip - 2eme info utile
            champAdr = 6;
        else if(champAdr == 6 && msg[i] != ')')//recuperation de l info
        {
            cp2[p2] = msg[i];
            p2++;
        }
        else if(champAdr == 6 && msg[i] == ')')//on indique que le champ est fini en passant au 7eme (inexistant) mais permet d eviter de recuperer le '.' de fin de chaine
            champAdr++;
    }
    //ajout du caractere de fin de chaine
    cp1[p1] = 0;
    cp2[p2] = 0;

    //conversion de la string en int
    p1 = atoi(cp1);
    p2 = atoi(cp2);

    //liberation de la memoire
    free(cp1);
    free(cp2);

    //calcul du port
    p1 = (p1*256) + p2;

    return p1;
}

//Recupere le dossier courant grace a la reponse serveur
//(fait le trie dans la réponse serveur)
void recuperationDossierCourant(int size, char* s, char* dest)
{
    char dossierCourant[BUFSIZ];
    int lectureDossierCourant = 1;
    int i = 5;
    
    if(strncmp(s,"257",3) == 0) //code retour srv ok -> "PATHNAME" créée
    {
        while(lectureDossierCourant == 1)
        {
            if(s[i] == '"') //lecture du 2eme '"'
            {
                lectureDossierCourant = 0; //fin de la lecture
                dossierCourant[i-5] = '\0';
            }
            else
            {
                dossierCourant[i-5] = s[i];
                i++;
            }
        }
    }
    else
        printf("PROBLEME LECTURE DOSSIER COURANT\n");

    size = i-5;

    for(i=0;i<size;i++)
        dest[i] = dossierCourant[i];
}

//Recupere la commande utilisateur (donc le premier mot)
void recuperationCommande(char* entry_user, char* cmd)
{
    int i=0;
    char clu = entry_user[i];

    //tant qu on a pas trouve l espace on continue la lecture
    while(clu != ' ')
    {
        cmd[i] = entry_user[i]; //recopie lettre par lettre de la commande
        i++;
        clu = entry_user[i];//caractere suivant
    }

    //ajout du caractere de fin de chaine a la commande
    cmd[i] = '\0';
}

//recupere la substring voulu
char *subcommande(char* string, int numSubARecup, char separateur)
{
    char *subcmd = (char*)malloc((BUFSIZ)*sizeof(char*));
    int size = strlen(string);
    int numCharEnregistre =0;
    int teteLecture=0;
    int numSub = 0;


    //lecture de la string
    for(;teteLecture<=size;teteLecture++)
    {
        //si c est pas le separateur alors
        if(string[teteLecture] != separateur)
        {
            //si ce n est pas la substring a recuperer n a pas ete prise
            if(numSub < numSubARecup)
            {
                //enregistrement char par char de l argument
                subcmd[numCharEnregistre] = string[teteLecture];
                //incrementation pour enregistrer prochain char
                numCharEnregistre++;
            }      
        }
        //si on a un separateur
        else
        {
            subcmd[numCharEnregistre] = 0;
            //on remet a 0 pour enregistrer le prochain char
            numCharEnregistre=0;
            //on a capture un argument - attraper les tous
            numSub++;
        }
    }
    //printf("SUBCOMMANDE - RETURN SUBCOMMANDE : %s\n",subcmd);
    //retour de l argument voulu
    return subcmd;
}

int getnbarguments(char* string, char separateur)
{
    int size = strlen(string);
    int i=0;
    int nbarg = 1;

    for(;i<=size;i++)
    {
        if(string[i] == separateur && i+1 <= size)
        {
            nbarg++;
        }
    }

    return nbarg;
}

struct listeArgument recuperationArguments(char* entry_user)
{
    //declaration structure
    struct listeArgument listeArguments;
    //allocation liste arguments
    listeArguments.arguments = (char**)malloc((BUFSIZ) * sizeof(char*));
    //string qui va recevoir arguments
    char *arg;

    int i=0;
    int init = 0;
    
    //recuperation nb arguments
    listeArguments.nbarg = getnbarguments(entry_user,' ');
    
    for(;i<listeArguments.nbarg;i++)
    {
        //recuperation de l argument
        arg = subcommande(entry_user,i+1,' ');
        //allocation de la memoire pour l argument
        listeArguments.arguments[i] = (char*)malloc((BUFSIZ)*sizeof(char));
        //initialisation de la memoire
        for(;init<BUFSIZ;init++)
            listeArguments.arguments[init] = 0;
        //ajout de l argument
        listeArguments.arguments[i] = arg;
    }

    return listeArguments;
}


char* commandeToUpper(char* cmd)
{
    int i=0;
    //recuperation de la commande
    int size = strlen(subcommande(cmd,1,' '));

    for(;i<size;i++)
    {
        //mis en majuscule de la commande
        cmd[i] = toupper((int)(cmd[i]));
    }
    return cmd;
}


int getnumcmd(char* cmd)
{
    int num=-1;
    char* c;// = (char*)malloc((BUFSIZ)*sizeof(char*));
    //printf("GETNUMCMD - SUBCOMMANDE : %s\n",subcommande(cmd,1,' '));
    //strcpy(c,subcommande(cmd,1,' '));
    
    c = subcommande(cmd,1,' ');
    
    c = commandeToUpper(c);
    //printf("GETNUMCMD - COMMANDE : %s\n",c); //pour moi (lois)

    //on compare les strings et on donne la valeur de la struct si ok
    if(strcmp(c,"HELP") == 0)
        num = HELP;
    else if(strcmp(c,"CD") == 0)
        num = CD;
    else if(strcmp(c,"LCD") == 0)
        num = LCD;
    else if(strcmp(c,"PWD") == 0)
        num = PWD;
    else if(strcmp(c,"LPWD") == 0)
        num = LPWD;
    else if(strcmp(c,"QUIT") == 0)
        num = QUIT;
    else if(strcmp(c,"LS") == 0)
        num = LS;
    else if(strcmp(c,"LLS") == 0)
        num = LLS;
    else if(strcmp(c,"GET") == 0)
        num = GET;
    else if(strcmp(c,"PUT") == 0)
        num = PUT;
    else if(strcmp(c,"MKDIR") == 0)
        num = PUT;
    else if(strcmp(c,"LMKDIR") == 0)
        num = LMKDIR;
    else if(strcmp(c, "RM") == 0)
        num = RM;
    else if(strcmp(c,"LRM") == 0)
        num = LRM;
    else if(strcmp(c,"PASV") == 0)
        num = PASVMODE;

    free (c);
    return num;
}

//modifie cmd a envoyer pour qu elle soit reconnu
//modification pour la cmd cd
char* cmd_cd(char* cmd, struct listeArgument listearg)
{
    int i=0;
    strcpy(cmd,"");

    //modification de la commande pour qu elle soit reconnu par le serveur
    strcpy(listearg.arguments[0],"CWD");

    for(;i<listearg.nbarg;i++)
    {
        //on recre la commande
        strcat(cmd,listearg.arguments[i]);
        if(i+1 < listearg.nbarg)
            strcat(cmd, " ");
    }
    
    return cmd;
}

//Affiche repertoire courant en local
void cmd_lpwd()
{
    char cwd[1024];
    printf("\"%s\" est le dossier courrant en local\n",getcwd(cwd,sizeof(cwd)));
}

//change dossier courant en local
void cmd_lcd(char* newWorkinkDirectory)
{
    if(chdir(newWorkinkDirectory) != 0)
    {
        perror("Erreur changement dossier.\n");
    }
    else
    {
        //on indique le nouveau dossier courant en local
        cmd_lpwd();
    }
}

void cmd_lls(struct listeArgument listeArg)
{
    //si le client liste autre chose que le dossier courant
    if(listeArg.nbarg > 1)
    {
        //allocation pour la commande
        char* cmd = (char*)malloc((BUFSIZ)*sizeof(char*));
        //modification pour qu elle soit reconnu
        strcpy(cmd,"ls -l ");
        //recreation de la commande
        strcat(cmd,listeArg.arguments[1]);
        //envoie commande au systeme
        system(cmd);
        //liberation memoire
        free(cmd);
    }
    else
    {
        //si ls dans le dossier courant
        system("ls -l");
    }
}

//creation socket
int creaSock(char* ip, int port)
{
    int newsock;
    struct sockaddr_in nServAdr;
    struct hostent *recup;

    newsock = socket(AF_INET,SOCK_STREAM,0);
    if(newsock <= ERREUR)
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

    //Integration de l adresse serveur dans la socket cliente
    memcpy((char *)&nServAdr.sin_addr, (char *)recup->h_addr, recup->h_length);
    nServAdr.sin_family = AF_INET;
    nServAdr.sin_port = htons((u_short) port);//Ajout du numero du port

    //Connection
    if(connect(newsock,(struct sockaddr *)&nServAdr,sizeof(nServAdr)) == ERREUR)
    {
        perror("Erreur connexion");
        exit(errno);
    }
    
    return newsock;
}

//ls sur serveur
void cmd_ls(char* ip, char* cmd, int sock)
{
    //Variables
    int verifLecture;
    int port;
    int newsock; //socket pour la connection passive

    char msgSrv[BUFSIZ];
    char msgPourSrv[BUFSIZ];

    //Initialisation
    //pour les retours du serveur
    int i = 0;
    for(;i<BUFSIZ;i++)
    {
        msgSrv[i] = '.';
        msgPourSrv[i] = '.';
    }
    
    //passage en mode binaire pour effectuer le transfert
    if(write(sock,"TYPE I\r\n",8) == ERREUR)
    {
        perror("Erreur TYPE I");
        return;//c est moche ENCORE oui mais faut sortir :(
    }
    
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
    
    //demande pour utiliser serveur en mode passif
    if(write(sock,"PASV\r\n",6) == ERREUR)
    {
        perror("Erreur mode passif");
        return;
    }
    
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
    
    port = getportpasv(verifLecture,msgSrv);
    
    //creation socket pour la connection data
    newsock = creaSock(ip,port);

    sprintf(msgPourSrv,"LIST\r\n");

    //demande de lister fichiers et dossiers
    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur mode passif");
        return;
    }

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    //fichier temporaire
    //int tmpfile = open("temp.txt",O_WRONLY|O_CREAT|O_TRUNC);

    //tant qu on a quelque chose a lire
    while((verifLecture = read(newsock,msgSrv,strlen(msgSrv))) > 0)
    {
        //ecriture dans le fichier du message du nombre d octet recu
        //write(tmpfile,msgSrv,verifLecture);
        afficheReponse(verifLecture,msgSrv);
    }

    //fermeture fichier
    //close(tmpfile);

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    //fermeture socket pour passif
    close(newsock);
}

//telechargement fichier
void cmd_get(char* ip, struct listeArgument listeArg, int sock)
{
    //Variables
    int verifLecture;
    int port;
    int newsock; //socket pour la connection passive
    
    //pour avancee
    int sizeFile;
    int tmp;
    int tmp2;
    int p;
    int total;
    int down = 2;

    char msgSrv[BUFSIZ];
    char msgPourSrv[BUFSIZ];

    //pour fichier local
    int currentSize;
    int f;
    

    //Initialisation
    //pour les retours du serveur
    int i = 0;
    for(;i<BUFSIZ;i++)
    {
        msgSrv[i] = '.';
        msgPourSrv[i] = '.';
    }

    //passage en mode binaire pour effectuer le transfert
    if(write(sock,"TYPE I\r\n",8) == ERREUR)
    {
        perror("Erreur TYPE I");
        return;//c est moche ENCORE oui mais faut sortir :(
    }
    
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

     //demande pour utiliser serveur en mode passif
    if(write(sock,"PASV\r\n",6) == ERREUR)
    {
        perror("Erreur mode passif");
        return;
    }
    
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
    
    port = getportpasv(verifLecture,msgSrv);
    
    //creation socket pour la connection data
    newsock = creaSock(ip,port);

    //recuperation taille du fichier par le serveur
    //on emet l hypothese qu il n y a qu un seul argument
    sprintf(msgPourSrv,"SIZE %s\r\n",listeArg.arguments[1]);

    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur recuperation taille fichier");
        return;
    }

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    //afficheReponse(verifLecture,msgSrv);
    sizeFile = atoi(msgSrv+4);

    //recuperation fichier
    sprintf(msgPourSrv,"RETR %s\r\n",listeArg.arguments[1]);

    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur demande telechargement");
        return;
    }

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }

    //---------------------------------
    //Recuperation fichier

    //creation fichier
    f = open(listeArg.arguments[1],O_CREAT|O_WRONLY|O_TRUNC,0644);
    //pour avancement
    currentSize = 0;
    if(sizeFile%100 == 0) //car 100% = fichier telecharge
        tmp = (sizeFile/100);
    else
        tmp = (sizeFile/100)+1;

    tmp2 = tmp;
    printf("Telechargement [");//Debut avance
    fflush(stdout);
    
    //tant qu on recoit quelque chose
    while((verifLecture = read(newsock,msgSrv,BUFSIZ)) > 0)
    {
        //indicateur d ecriture pour la reception actuelle
        total = 0;
        //ou on en est dans la taille total transfere
        currentSize += verifLecture;
        tmp2 = tmp * down;
        //boucle pour la progression
        while(tmp2 <= currentSize)
        {
            printf("#");
            fflush(stdout);
            down += 2;
            tmp2 = tmp * down;
        }
        
        //tant qu on a pas ecrit tout ce qu on a recu
        while(total < verifLecture)
        {
            //ecrite dans le fichier de ce qu on a recu
            p = write(f,msgSrv+total,verifLecture-total);
            //mis a jour de la quantite recopiee
            total += p;
        }
    }

    printf("] 100%%\n");
    fflush(stdout);

    
    close(newsock);
    close(f);
    
    memset(msgSrv,'.',BUFSIZ);
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
    
}

void cmd_help()
{
    printf("help\tListe les commandes disponible\n");    
    printf("ls\tListe les fichiers du dossier courant sur le serveur\n");
    printf("lls\tListe les fichiers du dossier courant en local\n");
    printf("cd\tChange de dossier sur le serveur\n");
    printf("lcd\tChange de dossier en local\n");
    printf("pwd\tIndique dans quel dossier on se situe sur le serveur\n");
    printf("lpwd\tIndique dans quel dossier on se situe en local\n");
    printf("rm (non implemente)\t Permet de supprimer le fichier sur le serveur\n");
    printf("lrm (non implemente)\tPermet de supprmer le fichier local\n");
    printf("mkdir (non implemente)\tPermet de créer un dossier sur le serveur\n");
    printf("lmkdir (non implemente)\tPermet de créer un dossier en local\n");
    printf("get\tPermet de telecharger un fichier du serveur en local\n");
    printf("put\tPermet d'envoyer un fichier local sur le serveur\n");
    printf("pasv\tPermet de passer en mode passif ou en mode actif\n");
    printf("quit\tPermet de quitter le client ftp\n");
}

void cmd_put(char* ip, struct listeArgument listeArg, int sock)
{
    //Variables
    int verifLecture;
    int port;
    int newsock; //socket pour la connection passive

    int f; //pour fichier
    struct stat statFichier;//informations a propos du fichier

    //pour transfert
    int sizeFichier;
    int currentSize;
    int down = 2;
    int tmp;
    int tmp2;
    int total;
    int p;

    char msgSrv[BUFSIZ];
    char msgPourSrv[BUFSIZ];

    //Initialisation
    //pour les retours du serveur
    int i = 0;
    for(;i<BUFSIZ;i++)
    {
        msgSrv[i] = '.';
        msgPourSrv[i] = '.';
    }

    //passage en mode binaire pour effectuer le transfert
    if(write(sock,"TYPE I\r\n",8) == ERREUR)
    {
        perror("Erreur TYPE I");
        return;//c est moche ENCORE oui mais faut sortir :(
    }
    
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

     //demande pour utiliser serveur en mode passif
    if(write(sock,"PASV\r\n",6) == ERREUR)
    {
        perror("Erreur mode passif");
        return;
    }
    
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
    
    port = getportpasv(verifLecture,msgSrv);
    
    //creation socket pour la connection data
    newsock = creaSock(ip,port);
    
    //on emet l hypothese qu il n y a qu un seul argument
    sprintf(msgPourSrv,"STOR %s\r\n",listeArg.arguments[1]);

    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur upload");
        return;
    }

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }

    //------------------------------------
    //DEBUT ENVOIE
    f = open(listeArg.arguments[1], O_RDONLY);
    fstat(f,&statFichier);
    sizeFichier = (int)statFichier.st_size;
    //pour avancement
    currentSize = 0;
    if(sizeFichier%100 == 0)
        tmp = (sizeFichier/100);
    else
        tmp = (sizeFichier/100);
    tmp2 = tmp;
    printf("Envoie [");
    fflush(stdout);

    //tant qu on a pas envoye tout le fichier on continue
    while(sizeFichier > 0)
    {
        verifLecture = read(f,msgSrv,BUFSIZ);
        currentSize += verifLecture;
        tmp2 = tmp * down;

        while(tmp2 <= currentSize)
        {
            printf("#");
            fflush(stdout);
            down += 2;
            tmp2 = tmp * down;
        }
        total = 0;
        while(total < verifLecture)
        {
            //envoie des données
            p = write(newsock,msgSrv+total,verifLecture-total);
            total += p;
        }
        sizeFichier -= verifLecture;
    }

    printf("] 100 %%\n");
    fflush(stdout);
    

    close(newsock);

    //reception informations serveur transfert fini
    memset(msgSrv,'.',BUFSIZ);printf("coucou");fflush(stdout);
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
}

int creersockActif( u_short port) {

    int sock, retour;
    struct sockaddr_in adresse;
    
    sock = socket(AF_INET,SOCK_STREAM,0);

    if (sock<0) {
        perror ("ERREUR OUVERTURE");
        return(-1);
    }
  
    adresse.sin_family = AF_INET;
    adresse.sin_port = htons(port);
    adresse.sin_addr.s_addr=INADDR_ANY;
  
    retour = bind (sock,(struct sockaddr *)&adresse,sizeof(adresse));
    
    if (retour<0) {
        perror ("IMPOSSIBLE DE NOMMER LA SOCKET");
        return(-1);
    }

    return (sock);
}

void cmd_ls_pasv(char* iptmp, int sock)
{
    //Variables
    int verifLecture;
    int sockactif;
    int newsockactif;

    char msgSrv[BUFSIZ];
    char msgPourSrv[BUFSIZ];

    //pour travailler sur une variable local et pas changer l adresse
    char ip[BUFSIZ];
    strcpy(ip,iptmp);
    
    //Choix du port
    //Initialisation
    //pour les retours du serveur
    int i = 0;
    for(;i<BUFSIZ;i++)
    {
        msgSrv[i] = '.';
        msgPourSrv[i] = '.';
    }
    
    //verification ip et transformation si besoin
    if(strcmp(ip,"localhost") == 0)
        strcpy(ip,"127,0,0,1");
    else //sinon remplacer les '.' par des ','
    {
        for(i=0;i<9;i++)
        {
            if(ip[i] == '.')
                ip[i] = ',';
        }
    }

    //dynamiser le port ?
    strcat(ip,",136,202");
    sprintf(msgPourSrv,"PORT %s\r\n",ip);
    
    sockactif = creersockActif((136*256+202));
    listen(sockactif,5);
    
    //envoie de l information pour le numero du port
    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur PORT");
        return;//c est moche ENCORE oui mais faut sortir :(
    }
    
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
    
    if(write(sock,"LIST\r\n",6) == 0)
    {
        perror("LIST ACTIF");
        return;
    }

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
    
    newsockactif = accept (sockactif, (struct sockaddr *) 0, (unsigned int*) 0);
    if (newsockactif == -1) {
      perror("Erreur accept\n");
      return;
    }

    //lecture reponse
    verifLecture = read(newsockactif,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    //fin
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    close(sockactif);
    close(newsockactif);
}

void cmd_get_pasv(char* iptmp, struct listeArgument listeArg, int sock)
{
    //Variables
    int verifLecture;
    int sockactif;
    int newsockactif;
    
    //pour avancee
    int sizeFile;
    int tmp;
    int tmp2;
    int p;
    int total;
    int down = 2;

    char msgSrv[BUFSIZ];
    char msgPourSrv[BUFSIZ];

    //pour fichier local
    int currentSize;
    int f;
    

    //pour travailler sur une variable local et pas changer l adresse
    char ip[BUFSIZ];
    strcpy(ip,iptmp);
    
    //Choix du port
    //Initialisation
    //pour les retours du serveur
    int i = 0;
    for(;i<BUFSIZ;i++)
    {
        msgSrv[i] = '.';
        msgPourSrv[i] = '.';
    }
    
    //verification ip et transformation si besoin
    if(strcmp(ip,"localhost") == 0)
        strcpy(ip,"127,0,0,1");
    else //sinon remplacer les '.' par des ','
    {
        for(i=0;i<9;i++)
        {
            if(ip[i] == '.')
                ip[i] = ',';
        }
    }

    //dynamiser le port ?
    strcat(ip,",136,203");
    sprintf(msgPourSrv,"PORT %s\r\n",ip);
    
    sockactif = creersockActif((136*256+203));
    listen(sockactif,5);

    //envoie de l information pour le numero du port
    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur PORT");
        return;//c est moche ENCORE oui mais faut sortir :(
    }
    
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    //recuperation taille du fichier par le serveur
    //on emet l hypothese qu il n y a qu un seul argument
    sprintf(msgPourSrv,"SIZE %s\r\n",listeArg.arguments[1]);

    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur recuperation taille fichier");
        return;
    }

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    sizeFile = atoi(msgSrv+4);//3 char pour valeur reponse srv + 1 char pour espace

    //recuperation fichier
    sprintf(msgPourSrv,"RETR %s\r\n",listeArg.arguments[1]);

    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur demande telechargement");
        return;
    }

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }

    //---------------------------------
    //Recuperation fichier
    
    newsockactif = accept (sockactif, (struct sockaddr *) 0, (unsigned int*) 0);
    if (newsockactif == -1) {
      perror("Erreur accept\n");
      return;
    }

    //creation fichier
    f = open(listeArg.arguments[1],O_CREAT|O_WRONLY|O_TRUNC,0644);
    //pour avancement
    currentSize = 0;
    if(sizeFile%100 == 0) //car 100% = fichier telecharge
        tmp = (sizeFile/100);
    else
        tmp = (sizeFile/100)+1;

    tmp2 = tmp;
    printf("Telechargement [");//Debut avance
    fflush(stdout);
    
    //tant qu on recoit quelque chose
    while((verifLecture = read(newsockactif,msgSrv,BUFSIZ)) > 0)
    {
        //indicateur d ecriture pour la reception actuelle
        total = 0;
        //ou on en est dans la taille total transfere
        currentSize += verifLecture;
        tmp2 = tmp * down;
        //boucle pour la progression
        while(tmp2 <= currentSize)
        {
            printf("#");
            fflush(stdout);
            down += 2;
            tmp2 = tmp * down;
        }
        
        //tant qu on a pas ecrit tout ce qu on a recu
        while(total < verifLecture)
        {
            //ecrite dans le fichier de ce qu on a recu
            p = write(f,msgSrv+total,verifLecture-total);
            //mis a jour de la quantite recopiee
            total += p;
        }
    }

    printf("] 100%%\n");
    fflush(stdout);

    close(sockactif);
    close(newsockactif);
    close(f);
    
    memset(msgSrv,'.',BUFSIZ);
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
}

void cmd_put_pasv(char* iptmp, struct listeArgument listeArg, int sock)
{
    //Variables
    int verifLecture;
    int sockactif;
    int newsockactif;

    int f; //pour fichier
    struct stat statFichier;//informations a propos du fichier

    //pour transfert
    int sizeFichier;
    int currentSize;
    int down = 2;
    int tmp;
    int tmp2;
    int total;
    int p;

    char msgSrv[BUFSIZ];
    char msgPourSrv[BUFSIZ];

    //pour travailler sur une variable local et pas changer l adresse
    char ip[BUFSIZ];
    strcpy(ip,iptmp);
    
    //Choix du port
    //Initialisation
    //pour les retours du serveur
    int i = 0;
    for(;i<BUFSIZ;i++)
    {
        msgSrv[i] = '.';
        msgPourSrv[i] = '.';
    }
    
    //verification ip et transformation si besoin
    if(strcmp(ip,"localhost") == 0)
        strcpy(ip,"127,0,0,1");
    else //sinon remplacer les '.' par des ','
    {
        for(i=0;i<9;i++)
        {
            if(ip[i] == '.')
                ip[i] = ',';
        }
    }

    //dynamiser le port ?
    strcat(ip,",136,204");
    sprintf(msgPourSrv,"PORT %s\r\n",ip);
    
    sockactif = creersockActif((136*256+204));
    listen(sockactif,5);
    
    //envoie de l information pour le numero du port
    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur PORT");
        return;//c est moche ENCORE oui mais faut sortir :(
    }

    //on emet l hypothese qu il n y a qu un seul argument
    sprintf(msgPourSrv,"STOR %s\r\n",listeArg.arguments[1]);

    if(write(sock,msgPourSrv,strlen(msgPourSrv)) == ERREUR)
    {
        perror("Erreur upload");
        return;
    }

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    //------------------------------------
    //DEBUT ENVOIE

    newsockactif = accept (sockactif, (struct sockaddr *) 0, (unsigned int*) 0);
    if (newsockactif == -1) {
      perror("Erreur accept\n");
      return;
    }

    f = open(listeArg.arguments[1], O_RDONLY);
    fstat(f,&statFichier);
    sizeFichier = (int)statFichier.st_size;
    //pour avancement
    currentSize = 0;
    if(sizeFichier%100 == 0)
        tmp = (sizeFichier/100);
    else
        tmp = (sizeFichier/100);
    tmp2 = tmp;
    printf("Envoie [");
    fflush(stdout);

    //tant qu on a pas envoye tout le fichier on continue
    while(sizeFichier > 0)
    {
        verifLecture = read(f,msgSrv,BUFSIZ);
        currentSize += verifLecture;
        tmp2 = tmp * down;

        while(tmp2 <= currentSize)
        {
            printf("#");
            fflush(stdout);
            down += 2;
            tmp2 = tmp * down;
        }
        total = 0;
        while(total < verifLecture)
        {
            //envoie des données
            p = write(newsockactif,msgSrv+total,verifLecture-total);
            total += p;
        }
        sizeFichier -= verifLecture;
    }
    
    printf("] 100 %%\n");
    fflush(stdout);

    close(sockactif);
    close(newsockactif);

    //reception informations serveur transfert fini
    memset(msgSrv,'.',BUFSIZ);
    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);

    verifLecture = read(sock,msgSrv,strlen(msgSrv));
    if(verifLecture <= ERREUR)
    {
        perror("Erreur read");
        exit(errno);
    }
    afficheReponse(verifLecture,msgSrv);
}
