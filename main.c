#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gamelib.h"
int main(){
    printf("\033[2J\033[31;40m");
    time_t t;
    srand((unsigned)time(&t));
    printf("\n--------------------Benvenuti-in--------------------\n\033[1m\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        \n           XXXX     XXX     XXXXX   XXXXXXX         \n          X    X   X   X   X     X  X               \n         X        X     X  X        X               \n         X        X     X   XXXXX   XXXX            \n         X        X     X        X  X               \n          X    X   X   X   X     X  X               \n           XXXX     XXX     XXXXX   XXXXXXX         \n                                                    \n XXXXX   XXXXXXX  XXXXXX      X     X     X  XXXXXXX\nX     X     X     X     X    X X    XX    X  X      \nX           X     X     X    X X    X X   X  X      \n XXXXX      X     XXXXXX    X   X   X  X  X  XXXX   \n      X     X     X   X     XXXXX   X   X X  X      \nX     X     X     X    X   X     X  X    XX  X      \n XXXXX      X     X     X  X     X  X     X  XXXXXXX\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n\033[22m\n");
    short scelta=0;
    short validInput;
    do{
        do{
            printf("------------------------Menù------------------------\n1)Imposta gioco\n2)Gioca\n3)Termina gioco\n4)Crediti\nScegli un opzione del menu:");
            validInput=scanf("%hd",&scelta);
            if(validInput!=1){
                printf("Non hai scelto un opzione valida\n");
                while(getchar()!='\n');
            }
            printf("----------------------------------------------------\n\n");
        }while(validInput!=1);
        switch(scelta){
            case 1:
                imposta_gioco();
                break;
            case 2:
                gioca();
                break;
            case 3:
                char sure;
                while(getchar()!='\n');
                printf("Sei sicuro di voler terminare?[y/n]:");
                scanf("%c",&sure);
                if(sure=='y'){
                    termina_gioco();
                    return 0;
                }
                break;
            case 4:
                crediti();
                break;
            default:
                printf("Non hai scelto un opzione valida\n");
                break;
        }
    }
    while(1);
}