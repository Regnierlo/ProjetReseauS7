#ifndef FONCTIONCSCMD_H
#define FONCTIONCSCMD_H

void afficheReponse(int length, char *msgSrv);
//Recupere le dossier courant grace a la reponse serveur
//(fait le trie dans la réponse serveur)
void recuperationDossierCourant(int size, char* s,char* dest);
void recuperationCommande(char* entry_user, char* cmd);
struct listeArgument recuperationArguments(char* entry_user);
//recupere la substring voulu
char* subcommande(char* string, int numSubARecup, char separateur);
//recupere le nombre d arguments
int getnbarguments(char* string, char separateur);
//met en majuscule la commande (1er argument)
char* commandeToUpper(char* cmd);
//recupere numero commande grace a enum
int getnumcmd(char* cmd);
//recupere le port pour la connexion en passif
int getportpasv(int size, char* msg);
//creation socket
int creaSock(char* ip, int port);
//creation socket pour mode actif
int creersockActif( u_short port);
//-------------------------------
//gestion commande cd
char* cmd_cd(char* cmd, struct listeArgument listearg);
void cmd_lcd(char* newWorkinkDirectory);
void cmd_lpwd();
void cmd_ls(char* ip, char* cmd, int sock);
void cmd_lls(struct listeArgument listeArg);
void cmd_get(char* ip, struct listeArgument listeArg, int sock);
void cmd_help();
void cmd_put(char* ip, struct listeArgument listeArg, int sock);
void cmd_ls_activ(char* iptmp, int sock);
void cmd_get_activ(char* iptmp, struct listeArgument listeArg, int sock);
void cmd_put_activ(char* iptmp, struct listeArgument listeArg, int sock);
#endif