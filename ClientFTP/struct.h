#ifndef STRUCT_H
#define STRUCT_H

struct listeArgument{
    char** arguments;
    int nbarg;
};

typedef enum numcmd{
    HELP = 0,
    CD = 1,
    LCD = 2,
    PWD = 3,
    LPWD = 4,
    LS = 5,
    LLS = 6,
    MKDIR = 7,
    LMKDIR = 8,
    GET = 9,
    PUT = 10,
    RM = 11,
    LRM = 12,
    QUIT = 13
}enumcommande;

#endif