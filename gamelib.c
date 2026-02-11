#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "gamelib.h"
static short quantitaGiocatori=0,mapSize=0,giocatoreAttuale=0,mosso=0;
static struct giocatore *giocatori;
static struct zona_mondoreale *mappa_mondoreale;
static struct zona_soprasotto *mappa_soprasotto;
struct score{
    char vincitore[63];
    short mosse;
    short hp;
};
static struct score *scores;
static const char *tipoZona[]={"Bosco","Scuola","Laboratorio","Caverna","Strada","Giardino","Supermercato","Centrale elettrica","Deposito abbandonato","Stazione di polizia"};
static const char *tipoNemico[]={"Billi","Nessuno","Democane","Demotorzone"};
static const char *tipoOggetto[]={"Nulla","Bicicletta","Maglietta fuocoinferno","Bussola","Schitarrata metallica"};
static void pulisciBuffer(short validInput){
    while(getchar()!='\n');
    if(!validInput){
        printf("Non hai scelto un opzione valida\n");
    }
}
static short checkNemicoPresente(){
    if(giocatori[giocatoreAttuale].pos_mondoreale->nemicoMondoReale->tipoNemico!=1 && !giocatori[giocatoreAttuale].mondo && !giocatori[giocatoreAttuale].pos_mondoreale->nemicoMondoReale->tramortito){
        return 1;
    }
    if(giocatori[giocatoreAttuale].pos_soprasotto->nemicoSoprasotto->tipoNemico!=1 && giocatori[giocatoreAttuale].mondo && !giocatori[giocatoreAttuale].pos_soprasotto->nemicoSoprasotto->tramortito){
        return 1;
    }
    return 0;
}
static short checkDemotorzone(){
    struct zona_soprasotto *zona_attuale=mappa_soprasotto;
    while(zona_attuale!=NULL){
        if(zona_attuale->nemicoSoprasotto->tipoNemico==3){
            return 1;
        }
        zona_attuale=zona_attuale->avanti;
    }
    return 0;
}
static void aggiungiStatsNemico(struct zona_mondoreale *zonaAttualeMR,struct zona_soprasotto *zonaAttualeSS){
    switch(zonaAttualeMR->nemicoMondoReale->tipoNemico){
            case 0:
                zonaAttualeMR->nemicoMondoReale->puntiVita=20;
                zonaAttualeMR->nemicoMondoReale->attacco=6;
                zonaAttualeMR->nemicoMondoReale->difesa=4;
                break;
            case 1:
                zonaAttualeMR->nemicoMondoReale->puntiVita=0;
                zonaAttualeMR->nemicoMondoReale->attacco=0;
                zonaAttualeMR->nemicoMondoReale->difesa=0;
                break;
            case 2:
                zonaAttualeMR->nemicoMondoReale->puntiVita=15;
                zonaAttualeMR->nemicoMondoReale->attacco=4;
                zonaAttualeMR->nemicoMondoReale->difesa=4;
                break;
            default:
                break;
    }
    switch(zonaAttualeSS->nemicoSoprasotto->tipoNemico){
            case 1:
                zonaAttualeSS->nemicoSoprasotto->puntiVita=0;
                zonaAttualeSS->nemicoSoprasotto->attacco=0;
                zonaAttualeSS->nemicoSoprasotto->difesa=0;
                break;
            case 2:
                zonaAttualeSS->nemicoSoprasotto->puntiVita=15;
                zonaAttualeSS->nemicoSoprasotto->attacco=4;
                zonaAttualeSS->nemicoSoprasotto->difesa=4;
                break;
            case 3:
                zonaAttualeSS->nemicoSoprasotto->puntiVita=40;
                zonaAttualeSS->nemicoSoprasotto->attacco=15;
                zonaAttualeSS->nemicoSoprasotto->difesa=10;
                break;
            default:
                break;
    }
}
static void checkGiocatore(){
    if(giocatori[giocatoreAttuale].puntiVita<1){
        printf("%s è morto\n",giocatori[giocatoreAttuale].nome);
        if(sizeof(giocatori)==sizeof(struct giocatore)){
            free(giocatori);
        }
        for(short i=giocatoreAttuale;i<quantitaGiocatori;i++){
            strcpy(giocatori[i].nome,giocatori[i+1].nome);
            giocatori[i].puntiVita=giocatori[i+1].puntiVita;
            giocatori[i].attacco_psichico=giocatori[i+1].attacco_psichico;
            giocatori[i].difesa_psichica=giocatori[i+1].difesa_psichica;
            giocatori[i].fortuna=giocatori[i+1].fortuna;
            giocatori[i].mondo=giocatori[i+1].mondo;
            giocatori[i].pos_mondoreale=giocatori[i+1].pos_mondoreale;
            giocatori[i].pos_soprasotto=giocatori[i+1].pos_soprasotto;
            for(short c=0;c<3;c++){
                giocatori[i].zaino[c]=giocatori[i+1].zaino[c];
            }
        }
        quantitaGiocatori-=1;
        giocatori=realloc(giocatori,quantitaGiocatori*sizeof(struct giocatore));
    }
}
static short distanzaDemotorzone(){
    struct zona_soprasotto *zonaAttuale=malloc(sizeof(struct zona_soprasotto));
    zonaAttuale=giocatori[giocatoreAttuale].pos_soprasotto;
    short i=0,trovato=0;
    while(zonaAttuale->avanti!=NULL){
        if(zonaAttuale->nemicoSoprasotto->tipoNemico==3){
            trovato=1;
            break;
        }
        i++;
        zonaAttuale=zonaAttuale->avanti;
    }
    if(!trovato){
        i=0;
        while(zonaAttuale->indietro!=NULL){
            if(zonaAttuale->nemicoSoprasotto->tipoNemico==3){
                trovato=1;
                break;
            }
            i--;
            zonaAttuale=zonaAttuale->indietro;
        }
    }
    return i;
}
static short d20(){
    return 1+(rand()%20);
}
static void attacca(short player){
    short tiro=d20(),roll=tiro+(giocatori[giocatoreAttuale].fortuna/2),danno=0;
    struct nemico *nemicoAttuale=malloc(sizeof(struct nemico));
    if(giocatori[giocatoreAttuale].mondo){
        nemicoAttuale=giocatori[giocatoreAttuale].pos_soprasotto->nemicoSoprasotto;
    }
    else{
        nemicoAttuale=giocatori[giocatoreAttuale].pos_mondoreale->nemicoMondoReale;
    }
    if(player){
        printf("Tiri il dado da 20 e esce %hd ma con la tua fortuna diventa %hd\n",tiro,roll);
        if((giocatori[giocatoreAttuale].attacco_psichico-(2*nemicoAttuale->difesa))<0 && roll<15){
            printf("Attacchi il nemico ma non gli fai alcun danno\n");
        }
        else{
            if(roll<=0){
                printf("Provi ad attaccatare mancando il bersaglio,\nma nel nomento dell'attacco colpisci un sasso che esplodendo ti infigge 1 danno\n");
                giocatori[giocatoreAttuale].puntiVita-=20;
            }
            if(roll>0 && roll<4){
                printf("Attacchi mancando il bersaglio e il nemico non subisce danni\n");
            }
            if(roll>3 && roll<9){
                danno=giocatori[giocatoreAttuale].attacco_psichico-(2*nemicoAttuale->difesa);
                printf("Attacchi prendendo di striscio il nemico \ne gli infliggi %hd danni\n",danno);
            }
            if(roll>8 && roll<15){
                danno=giocatori[giocatoreAttuale].attacco_psichico-(nemicoAttuale->difesa);
                printf("Attacchi prendendo il nemico in un punto non vitale \ne gli infliggi %hd danni\n",danno);
            }
            if(roll>14 && roll<20){
                danno=giocatori[giocatoreAttuale].attacco_psichico-round(nemicoAttuale->difesa/4);
                printf("Attacchi prendendo il nemico in un punto vitale \ne gli infliggi %hd danni\n",danno);
            }
            if(roll>19){
                danno=giocatori[giocatoreAttuale].attacco_psichico+round(nemicoAttuale->difesa/4);
                printf("Attacchi prendendo il nemico in un punto vitale\nfacendogli anche del danno critico aggiuntivo\ne gli infliggi %hd danni\n",danno);
            }
        }
        if(danno>=0){
            nemicoAttuale->puntiVita-=danno;
        }
    }
    else{
        if(tiro>0 && tiro<4){
            printf("Il nemico prova ad attaccarti mancandoti e non subisci danno\n");
        }
        if(tiro>3 && tiro<9){
            danno=nemicoAttuale->attacco-(giocatori[giocatoreAttuale].difesa_psichica);
            if(danno>=0){
                printf("Il nemico ti attacca prendendoti di striscio\ne ti infligge %hd danni\n",danno);
            }
            else{
                printf("Il nemico ti attacca ma non ti fa danno\n");
            }
        }
        if(tiro>8 && tiro<15){
            danno=nemicoAttuale->attacco-(giocatori[giocatoreAttuale].difesa_psichica/2);
            if(danno>=0){
                printf("Il nemico ti attacca prendendoti in un punto non vitale\ne ti infligge %hd danni\n",danno);
            }
            else{
                printf("Il nemico ti attacca ma non ti fa danno\n");
            }
        }
        if(tiro>14 && tiro<20){
            danno=nemicoAttuale->attacco-round(giocatori[giocatoreAttuale].difesa_psichica/4);
            if(danno>=0){
                printf("Il nemico ti attacca prendendoti in un punto vitale \ne ti infligge %hd danni\n",danno);
            }
            else{
                printf("Il nemico ti attacca ma non ti fa danno\n");
            }
        }
        if(tiro>19){
            danno=nemicoAttuale->attacco;
            printf("Il nemico ti attacca prendendoti in un punto vitale\nfacendoti anche del danno critico\ne ti infligge %hd danni\n",danno);
        }
        if(danno>=0){
            giocatori[giocatoreAttuale].puntiVita-=danno;
        }
    }
} 
static short schiva(short player){
    short tiro=d20();
    if(player){
        printf("Tiri il dado da 20 e esce il %hd ma con la tua fortuna diventa %hd\n",tiro,tiro+(giocatori[giocatoreAttuale].fortuna/2));
        tiro+=(giocatori[giocatoreAttuale].fortuna/2);
    }
    else{
        printf("Il nemico prova a schivare il tuo prossimo attacco ");
    }
    if(tiro>12){
        if(player){
            printf("Riesci a schivare il prossimo attacco nemico\n");
        }
        else{
            printf("e riesce\n");
        }
        return 1;
    }
    else{
        if(player){
            printf("Provi a schivare il prossimo attacco nemico ma non ce la fai\n");
        }
        else{
            printf("ma non riesce\n");
        }
        return 0;
    }
}
static void addScore(short turni){
    if(scores==NULL){
        scores=malloc(3*sizeof(struct score));
    }
    for(short i=2;i>0;i--){
        scores[i].mosse=scores[i-1].mosse;
        strcpy(scores[i].vincitore,scores[i-1].vincitore);
        scores[i].hp=scores[i-1].hp;
    }
    strcpy(scores[0].vincitore,giocatori[giocatoreAttuale].nome);
    scores[0].mosse=turni;
    scores[0].hp=giocatori[giocatoreAttuale].puntiVita;
}
static void avanza(){
    if(mosso){
        printf("Ti sei gia mosso\n");
        return;
    }
    if(checkNemicoPresente()){
        printf("Devi prima sconfiggere il nemico\n");
        return;
    }
    if(giocatori[giocatoreAttuale].pos_mondoreale->avanti!=NULL){
        if(giocatori[giocatoreAttuale].mondo){
            giocatori[giocatoreAttuale].pos_soprasotto->nemicoSoprasotto->tramortito=0;
        }
        else{
            giocatori[giocatoreAttuale].pos_mondoreale->nemicoMondoReale->tramortito=0;
        }
        mosso=1;
        giocatori[giocatoreAttuale].pos_mondoreale=giocatori[giocatoreAttuale].pos_mondoreale->avanti;
        giocatori[giocatoreAttuale].pos_soprasotto=giocatori[giocatoreAttuale].pos_soprasotto->avanti;
        printf("Sei avanazato di una zona\n");
    }
    else{
        printf("Non puoi andare avanti\n");
    }
}
static void indietreggia(){
    if(checkNemicoPresente()){
        printf("Devi prima sconfiggere il nemico\n");
        return;
    }
    if(giocatori[giocatoreAttuale].pos_mondoreale->indietro!=NULL){
        if(giocatori[giocatoreAttuale].mondo){
            giocatori[giocatoreAttuale].pos_soprasotto->nemicoSoprasotto->tramortito=0;
        }
        else{
            giocatori[giocatoreAttuale].pos_mondoreale->nemicoMondoReale->tramortito=0;
        }
        giocatori[giocatoreAttuale].pos_mondoreale=giocatori[giocatoreAttuale].pos_mondoreale->indietro;
        giocatori[giocatoreAttuale].pos_soprasotto=giocatori[giocatoreAttuale].pos_soprasotto->indietro;
        printf("Sei indietreggiato di una zona\n");
    }
    else{
        printf("Non puoi andare indietro\n");
    }
}
static void cambia_mondo(){
    if(mosso){
        printf("Ti sei gia mosso\n");
        return;
    }
    if(checkNemicoPresente() && !giocatori[giocatoreAttuale].mondo){
        printf("Devi prima sconfiggere il nemico\n");
        return;
    }
    if(checkNemicoPresente() && giocatori[giocatoreAttuale].mondo){
        short tiro=d20();
        printf("Tiri il dado da 20 e ");
        if(tiro>giocatori[giocatoreAttuale].fortuna){
            printf("%d troppo alto per scappare\n",tiro);
            return;
        }
        else{
            printf("%d perfetto per scappare\n",tiro);
        }
    }
    giocatori[giocatoreAttuale].mondo=!giocatori[giocatoreAttuale].mondo;
    printf("Sei andato nel ");
    if(giocatori[giocatoreAttuale].mondo){
        printf("Soprasotto");
    }
    else{
        printf("Mondoreale");
    }
    printf("\n");
    mosso=1;
}
static void utilizza_oggetto(){
    short validInput,scelta;
    do{
        for(short i=0;i<3;i++){
            if(giocatori[giocatoreAttuale].zaino[0]==0){
                printf("Non hai oggetti da utilizzare\n");
                return;
            }
            if(giocatori[giocatoreAttuale].zaino[i]!=0){
                printf("%hd)%s\n",i+1,tipoOggetto[giocatori[giocatoreAttuale].zaino[i]]);
            }
        }
        printf("Quale oggetto vuoi usare?: ");
        validInput=scanf("%hd",&scelta);
        pulisciBuffer(validInput);
    }while(validInput!=1);
    scelta--;
    switch(giocatori[giocatoreAttuale].zaino[scelta]){
        case 1:
            short roll=d20()+(giocatori[giocatoreAttuale].fortuna/2);
            if(roll>12){
                if(giocatori[giocatoreAttuale].mondo){
                    giocatori[giocatoreAttuale].pos_soprasotto->nemicoSoprasotto->tramortito=1;
                }
                else{
                    giocatori[giocatoreAttuale].pos_mondoreale->nemicoMondoReale->tramortito=1;
                }
                printf("Sei riuscito a scappare dal nemico");
            }
            else{
                printf("Non sei riuscito a scappare dal nemico");
            }
            printf(" ma la bicicletta si rompe");
            break;
        case 2:
            giocatori[giocatoreAttuale].puntiVita+=5;
            printf("Ti senti molto più in salute(+5 PV) ma la maglietta si strappa");
            break;
        case 3:
            if(giocatori[giocatoreAttuale].mondo){
                printf("Provi ad utilizzare la bussola ma siccome sei nel\nsoprasotto l'ago inizia a girare senza fermarsi finche non si rompe");
            }
            else{
                printf("La bussola ti dice che il demotorzone si trova a %hd\nzone di distanza da te ma subito dopo si rompe",distanzaDemotorzone());
            }
            break;
        case 4:
            giocatori[giocatoreAttuale].attacco_psichico+=2;
            giocatori[giocatoreAttuale].difesa_psichica+=2;
            giocatori[giocatoreAttuale].fortuna+=3;
            printf("Improvvisamente ti senti più forte più robusto e più\nfortunato ma le corde della chitarra si rompono");
            break;
        default:
            break;
    }
    printf("\n");
    giocatori[giocatoreAttuale].zaino[scelta]=0;
}
static void combatti(){
    if(checkNemicoPresente()){
        short schivaP=0,schivaN=0;
        struct nemico *nemicoAttuale=malloc(sizeof(struct nemico));
        if(!giocatori[giocatoreAttuale].mondo){
            nemicoAttuale=giocatori[giocatoreAttuale].pos_mondoreale->nemicoMondoReale;
        }
        else{
            nemicoAttuale=giocatori[giocatoreAttuale].pos_soprasotto->nemicoSoprasotto;
        }
        printf("----------------------------------------------------\n");
        short tiro=d20(),turno=0,validInput=0,scelta=0;
        printf("Tiri il dado da 20 e esce il %hd ma con la tua fortuna diventa %hd\n",tiro,tiro+(giocatori[giocatoreAttuale].fortuna/2));
        tiro+=(giocatori[giocatoreAttuale].fortuna/2);
        if(tiro<10){
            printf("Il nemico inizia per primo\n");
            turno=1;
        }
        else{
            printf("Inizi tu per primo\n");
            turno=0;
        }
        printf("\n");
        while(checkNemicoPresente() && nemicoAttuale->puntiVita>0 && !nemicoAttuale->tramortito && giocatori[giocatoreAttuale].puntiVita>0){
            printf("%s PV:%hd\n",tipoNemico[nemicoAttuale->tipoNemico],nemicoAttuale->puntiVita);
            if(turno==0){
                do{
                    printf("1)Attacca\n2)Schiva\n3)Zaino\n%s cosa vuoi fare?: ",giocatori[giocatoreAttuale].nome);
                    validInput=scanf("%hd",&scelta);
                    printf("\n");
                    pulisciBuffer(validInput);
                }while(validInput!=1);
                switch(scelta){
                    case 1:
                        if(!schivaN){
                            attacca(1);
                        }
                        else{
                            printf("Il nemico ha schivato il tuo attacco\n");
                            schivaN=0;
                        }
                        break;
                    case 2:
                        schivaP=schiva(1);
                        break;
                    case 3:
                        utilizza_oggetto();
                        break;
                    default:
                        turno=1;
                        printf("Non hai scelto un opzione disponibile\n");
                        break;
                }
            }
            else{
                scelta=1+(rand()%200);
                if(scelta<120){
                    if(!schivaP){
                        attacca(0);
                    }
                    else{
                        printf("Il nemico prova ad attaccarti ma tu schivi l'attacco\n");
                        schivaP=0;
                    }
                }
                else{
                    schivaN=schiva(0);
                }
            }
            printf("\n");
            turno=(turno+1)%2;
        }
        printf("----------------------------------------------------\n");
        if(nemicoAttuale->puntiVita<1){
            if(nemicoAttuale->tipoNemico==3){
                nemicoAttuale->tipoNemico=1;
            }
            else{
                if((rand()%100)%2==0){
                    nemicoAttuale->tipoNemico=1;
                }
                else{
                    nemicoAttuale->tramortito=1;
                }
            }
        }
        aggiungiStatsNemico(giocatori[giocatoreAttuale].pos_mondoreale,giocatori[giocatoreAttuale].pos_soprasotto);
    }
    else{
        printf("Non ci sono nemici in questa zona\n");
    }
}
static void stampa_giocatore(){
    printf("----------------------------------------------------\n%s\nPunti vita: %hd\nAttacco psichico: %hd\nDifesa psichica: %hd\nFortuna: %hd\nInventario:\n",giocatori[giocatoreAttuale].nome,giocatori[giocatoreAttuale].puntiVita,giocatori[giocatoreAttuale].attacco_psichico,giocatori[giocatoreAttuale].difesa_psichica,giocatori[giocatoreAttuale].fortuna);
    for(int i=0;i<3;i++){
        if(giocatori[giocatoreAttuale].zaino[i]!=0){
            printf("    %s\n",tipoOggetto[giocatori[giocatoreAttuale].zaino[i]]);
        }
        if(!giocatori[giocatoreAttuale].zaino[0]){
            printf("    Non hai oggetti nello zaino\n");
            break;
        }
    }
    if(!giocatori[giocatoreAttuale].mondo){
        printf("Sei nel mondo reale\n");
    }
    else{
        printf("Sei nel SopraSotto\n");
    }
    printf("----------------------------------------------------\n\n");
}
static void raccogli_oggetto(){
    if(checkNemicoPresente()){
        printf("Devi prima sconfiggere il nemico\n");
        return;
    }
    if(giocatori[giocatoreAttuale].zaino[2]){
        printf("Hai lo zaino pieno\n");
        return;
    }
    for(short i=0;i<3;i++){
        if(giocatori[giocatoreAttuale].zaino[i]==0){
            giocatori[giocatoreAttuale].zaino[i]+=giocatori[giocatoreAttuale].pos_mondoreale->tipo_oggetto_mondoreale;
            break;
        }
    }
    printf("Hai raccolto: %s\n",tipoOggetto[giocatori[giocatoreAttuale].pos_mondoreale->tipo_oggetto_mondoreale]);
    giocatori[giocatoreAttuale].pos_mondoreale->tipo_oggetto_mondoreale=0;
}
static short passa(short turni,short giocatoreIniziale){
    giocatoreAttuale=(giocatoreAttuale+1)%quantitaGiocatori;
    if(giocatoreAttuale==giocatoreIniziale){
        turni++;
    }
    mosso=0;
    return turni;
}
static void elimina_mappa(){
    if(mappa_mondoreale!=NULL && mappa_soprasotto!=NULL){
        struct zona_mondoreale *zonaAttuale=malloc(sizeof(struct zona_mondoreale));
        zonaAttuale=mappa_mondoreale;
        while(zonaAttuale!=NULL){
            zonaAttuale=zonaAttuale->avanti;
        }
        while(zonaAttuale!=NULL){
            zonaAttuale=zonaAttuale->indietro;
            free(zonaAttuale->avanti->link_soprasotto);
            free(zonaAttuale->avanti);
        }
        free(mappa_mondoreale);
        free(mappa_soprasotto);
        mappa_mondoreale=NULL;
        mappa_soprasotto=NULL;
        mapSize=0;
    }
}
static void inserisci_giocatore(){
    short validInput,undiciEsiste=0,esiste=0;
    printf("----------------Inserisci-giocatori-----------------\n");
    do{
        printf("Inserisci la quantita di giocatori che vuoi inserire[1-4]: ");
        validInput=scanf("%hd",&quantitaGiocatori);
        pulisciBuffer(validInput);
    }while(validInput!=1);
    printf("----------------------------------------------------\n\n");
    if(quantitaGiocatori<1 || quantitaGiocatori>4){
        printf("Quantita sbagliata\n\n");
        return;
    }
    giocatori=malloc(quantitaGiocatori*sizeof(struct giocatore));
    for(short i=0;i<quantitaGiocatori;i++){
        char aumentaAtt[3];
        char aumentaDif[3];
        char nome[63];
        printf("--------------------Giocatore-%hd--------------------\n",i+1);
        do{
            esiste=0;
            printf("Inserisci il nome (max 61 caratteri): ");
            scanf("%61s",nome);
            for(short i=0;i<quantitaGiocatori;i++){
                if(*nome==*giocatori[i].nome){
                    esiste=1;
                }
            }
            if(esiste){
                printf("Giocatore gia esistente\n");
            }
            else{
                strcpy(giocatori[i].nome,nome);
                esiste=0;
            }
        }while(esiste);
        giocatori[i].puntiVita=20;
        giocatori[i].attacco_psichico=d20();
        giocatori[i].difesa_psichica=d20();
        giocatori[i].fortuna=d20();
        printf("Attacco psichico: %hd\n",giocatori[i].attacco_psichico);
        printf("Difesa psichica: %hd\n",giocatori[i].difesa_psichica);
        printf("Fortuna: %hd\n",giocatori[i].fortuna);
        giocatori[i].mondo=0;
        pulisciBuffer(1);
        printf("Vuoi aumentare l'attacco psichico ma diminuire la difesa psichica di 3?[y/n]");
        scanf("%c",aumentaAtt);
        if(aumentaAtt[0]=='y'){
            giocatori[i].attacco_psichico+=3;
            giocatori[i].difesa_psichica-=3;
            printf("Attacco psichico: %hd\n",giocatori[i].attacco_psichico);
            printf("Difesa psichica: %hd\n",giocatori[i].difesa_psichica);
            printf("Fortuna: %hd\n",giocatori[i].fortuna);
        }
        pulisciBuffer(1);
        printf("Vuoi aumentare la difesa psichica ma diminuire l'attacco psichico di 3?[y/n]");
        scanf("%c",aumentaDif);
        if(aumentaDif[0]=='y'){
            giocatori[i].attacco_psichico-=3;
            giocatori[i].difesa_psichica+=3;
            printf("Attacco psichico: %hd\n",giocatori[i].attacco_psichico);
            printf("Difesa psichica: %hd\n",giocatori[i].difesa_psichica);
            printf("Fortuna: %hd\n",giocatori[i].fortuna);
        }
        pulisciBuffer(1);
        if(!undiciEsiste){
            char vuole[3];
            printf("Vuoi diventare UndiciVirgolaCinque?[y/n]:");
            scanf("%c",vuole);
            if(vuole[0]=='y'){
                strcpy(giocatori[i].nome,"UndiciVirgolaCinque");
                giocatori[i].attacco_psichico+=4;
                giocatori[i].difesa_psichica+=4;
                giocatori[i].fortuna-=7;
                undiciEsiste=1;
                printf("Attacco psichico: %hd\n",giocatori[i].attacco_psichico);
                printf("Difesa psichica: %hd\n",giocatori[i].difesa_psichica);
                printf("Fortuna: %hd\n",giocatori[i].fortuna);
            }
            pulisciBuffer(1);
        }
        printf("----------------------------------------------------\n\n");
    }
}
static void genera_mappa(){
    if(mappa_mondoreale!=NULL){
        elimina_mappa();
    }
    mapSize=16;
    mappa_mondoreale=malloc(sizeof(struct zona_mondoreale));
    mappa_soprasotto=malloc(sizeof(struct zona_soprasotto));
    struct nemico *nemicoMR=malloc(sizeof(struct nemico));
    struct nemico *nemicoSS=malloc(sizeof(struct nemico));
    mappa_mondoreale->link_soprasotto=mappa_soprasotto;
    mappa_soprasotto->link_mondoreale=mappa_mondoreale;
    mappa_mondoreale->nemicoMondoReale=nemicoMR;
    mappa_soprasotto->nemicoSoprasotto=nemicoSS;
    nemicoSS->tipoNemico=1;
    nemicoMR->tipoNemico=1;
    for(short i=0;i<mapSize-1;i++){
        struct zona_mondoreale *nuovaMR=malloc(sizeof(struct zona_mondoreale));
        struct zona_soprasotto *nuovaSS=malloc(sizeof(struct zona_soprasotto));
        struct nemico *nuovoNMR=malloc(sizeof(struct nemico));
        struct nemico *nuovoNSS=malloc(sizeof(struct nemico));
        nuovaMR->link_soprasotto=nuovaSS;
        nuovaSS->link_mondoreale=nuovaMR;
        nuovaMR->nemicoMondoReale=nuovoNMR;
        nuovaSS->nemicoSoprasotto=nuovoNSS;
        nuovoNMR->tipoNemico=rand()%3;
        nuovoNMR->tramortito=0;
        nuovoNSS->tramortito=0;
        nuovaMR->tipo_zona_mondoreale=rand()%10;
        nuovaMR->tipo_oggetto_mondoreale=rand()%5;
        nuovaSS->tipo_zona_soprasotto=nuovaMR->tipo_zona_mondoreale;
        do{
            nuovaSS->nemicoSoprasotto->tipoNemico=1+(rand()%3);
        }
        while(nuovaSS->nemicoSoprasotto->tipoNemico==3 && (checkDemotorzone() || i<6));
        if(i==0){
            nuovaMR->indietro=mappa_mondoreale;
            nuovaSS->indietro=mappa_soprasotto;
            mappa_mondoreale->avanti=nuovaMR;
            mappa_soprasotto->avanti=nuovaSS;
        }
        else{
            struct zona_mondoreale *zonaPrecedenteMR=malloc(sizeof(struct zona_mondoreale));
            struct zona_soprasotto *zonaPrecedenteSS=malloc(sizeof(struct zona_soprasotto));
            zonaPrecedenteMR=mappa_mondoreale;
            zonaPrecedenteSS=mappa_soprasotto;
            while(zonaPrecedenteMR->avanti!=NULL && zonaPrecedenteSS!=NULL){
                zonaPrecedenteMR=zonaPrecedenteMR->avanti;
                zonaPrecedenteSS=zonaPrecedenteSS->avanti;
            }
            nuovaMR->indietro=zonaPrecedenteMR;
            nuovaSS->indietro=zonaPrecedenteSS;
            zonaPrecedenteMR->avanti=nuovaMR;
            zonaPrecedenteSS->avanti=nuovaSS;
            if(!checkDemotorzone() && i==mapSize-1){
                nuovaSS->nemicoSoprasotto->tipoNemico=3;
            }
        }
        aggiungiStatsNemico(nuovaMR,nuovaSS);
    }
    for(short i=0;i<quantitaGiocatori;i++){
        giocatori[i].pos_mondoreale=mappa_mondoreale;
        giocatori[i].pos_soprasotto=mappa_soprasotto;
    }
    printf("Mappa generata\n\n");
}
static void inserisci_zona(){
    short posto,validInput;
    printf("-------------------Inserisci-zona-------------------\n");
    do{
        printf("In che posto vuoi inserire la zona?[1-%hd]: ",mapSize);
        validInput=scanf("%hd",&posto);
        pulisciBuffer(validInput);
    }while(validInput!=1);
    if(posto>0 && posto<=mapSize){
        struct zona_mondoreale *nuovaZonaMR=malloc(sizeof(struct zona_mondoreale));
        struct zona_soprasotto *nuovaZonaSS=malloc(sizeof(struct zona_soprasotto));
        struct nemico *nemicoMR=malloc(sizeof(struct nemico));
        struct nemico *nemicoSS=malloc(sizeof(struct nemico));
        nuovaZonaMR->link_soprasotto=nuovaZonaSS;
        nuovaZonaSS->link_mondoreale=nuovaZonaMR;
        nuovaZonaMR->tipo_zona_mondoreale=rand()%10;
        nuovaZonaSS->tipo_zona_soprasotto=nuovaZonaMR->tipo_zona_mondoreale;
        nuovaZonaMR->nemicoMondoReale=nemicoMR;
        nuovaZonaSS->nemicoSoprasotto=nemicoSS;
        posto--;
        struct zona_mondoreale *zonaPrecedente=malloc(sizeof(struct zona_mondoreale));
        zonaPrecedente=mappa_mondoreale;
        for(short i=0;i<posto;i++){
            zonaPrecedente=zonaPrecedente->avanti;
        }
        if(zonaPrecedente->avanti!=NULL){
            zonaPrecedente->avanti->indietro=nuovaZonaMR;
            zonaPrecedente->link_soprasotto->avanti->indietro=nuovaZonaSS;
            nuovaZonaMR->avanti=zonaPrecedente->avanti;
            nuovaZonaSS->avanti=zonaPrecedente->link_soprasotto->avanti;
        }
        nuovaZonaMR->indietro=zonaPrecedente;
        nuovaZonaSS->indietro=zonaPrecedente->link_soprasotto;
        zonaPrecedente->avanti=nuovaZonaMR;
        zonaPrecedente->link_soprasotto->avanti=nuovaZonaSS;
        mapSize++;
        short oggetto=5,nemico=3,nemicoss=4;
        do{
            printf("1)Bicicletta\n2)Maglietta FuocoInferno\n3)Bussola\n4)Schitarrata Metallica\n0)Nulla\nChe oggetto vuoi inserire?:");
            scanf("%hd",&oggetto);
        }
        while(oggetto<0 || oggetto>4);
        nuovaZonaMR->tipo_oggetto_mondoreale=oggetto;
        do{
            printf("\n1)Nulla\n2)Democane\n0)Billi\nQuale nemico vuoi inserire nel mondo reale?:");
            scanf("%hd",&nemico);
        }
        while(nemico<0 || nemico>2);
        nuovaZonaMR->nemicoMondoReale->tipoNemico=nemico;
        do{
            printf("\n1)Nulla\n2)Democane\n");
            if(!checkDemotorzone()){
                printf("3)Demotorzone\n");
            }
            printf("Quale nemico vuoi inserire nel soprasotto?:");
            scanf("%hd",&nemicoss);
        }
        while(nemicoss<0 || nemicoss>3);
        nuovaZonaSS->nemicoSoprasotto->tipoNemico=nemicoss;
        if(!checkDemotorzone()){
            struct zona_soprasotto *zonaAttuale=malloc(sizeof(struct zona_soprasotto));
            zonaAttuale=mappa_soprasotto;
            while(zonaAttuale!=NULL){
                zonaAttuale=zonaAttuale->avanti;
            }
            zonaAttuale->nemicoSoprasotto->tipoNemico=3;
            aggiungiStatsNemico(zonaAttuale->link_mondoreale,zonaAttuale);
        }
        aggiungiStatsNemico(nuovaZonaMR,nuovaZonaSS);
        printf("Zona inserita\n");
    }
    else{
        printf("Posto non diponibile\n");
    }
    printf("----------------------------------------------------\n\n");
}
static void cancella_zona(){
    if(mappa_mondoreale==NULL){
        printf("Genera la mappa prima\n\n");
        return;
    }
    short posto,validInput;
    printf("-------------------Cancella-zona--------------------\n");
    do{
        printf("Quale zona vuoi cancellare?[2-%hd]: ",mapSize);
        validInput=scanf("%hd",&posto);
        pulisciBuffer(validInput);
    }while(validInput!=1);
    posto--;
    if(posto>=1 && posto<mapSize){
        struct zona_mondoreale *zonaAttuale=malloc(sizeof(struct zona_mondoreale));
        zonaAttuale=mappa_mondoreale;
        for(short i=0;i<posto;i++){
            zonaAttuale=zonaAttuale->avanti;
        }
        zonaAttuale->indietro->avanti=zonaAttuale->avanti;
        zonaAttuale->link_soprasotto->indietro->avanti=zonaAttuale->link_soprasotto->avanti;
        if(zonaAttuale->avanti!=NULL){
            zonaAttuale->avanti->indietro=zonaAttuale->indietro;
            zonaAttuale->link_soprasotto->avanti->indietro=zonaAttuale->link_soprasotto->indietro;
        }
        else{
            zonaAttuale->indietro->avanti=NULL;
            zonaAttuale->link_soprasotto->indietro->avanti=NULL;
        }
        mapSize-=1;
        free(zonaAttuale);
        printf("Posto rimosso\n");
    }
    else{
        printf("Posto non diponibile\n");
    }
    if(!checkDemotorzone()){
        struct zona_soprasotto *zonaAttuale;
        zonaAttuale=malloc(sizeof(struct zona_soprasotto));
        zonaAttuale=mappa_soprasotto;
        while(zonaAttuale->avanti!=NULL){
            zonaAttuale=zonaAttuale->avanti;
        }
        zonaAttuale->nemicoSoprasotto->tipoNemico=3;
        aggiungiStatsNemico(zonaAttuale->link_mondoreale,zonaAttuale);
    }
    printf("----------------------------------------------------\n\n");
}
static void stampa_mappa(){
    if(mappa_mondoreale==NULL){
        printf("Devi prima generare la mappa\n\n");
        return;
    }
    short validInput,tipo;
    do{
        printf("Quale mappa vorresti vedere mondo reale[0] o soprasotto[1]?: ");
        validInput=scanf("%hd",&tipo);
        pulisciBuffer(validInput);
    }while(validInput!=1);
    if(tipo==1){
        struct zona_soprasotto *zonaAttuale=malloc(sizeof(struct zona_soprasotto));
        zonaAttuale=mappa_soprasotto;
        printf("\n------------------Mappa-soprasotto------------------\n");
        short i=0;
        while(zonaAttuale!=NULL){
            printf("Zona %hd\n%s\nNemico: %s\n\n",i+1,tipoZona[zonaAttuale->tipo_zona_soprasotto],tipoNemico[zonaAttuale->nemicoSoprasotto->tipoNemico]);
            i++;
            zonaAttuale=zonaAttuale->avanti;
        }
    }
    if(tipo==0){
        printf("\n------------------Mappa-mondoreale------------------\n");
        struct zona_mondoreale *zonaAttuale=malloc(sizeof(struct zona_mondoreale));
        zonaAttuale=mappa_mondoreale;
        short i=0;
        while(zonaAttuale!=NULL){
            printf("Zona %hd\n%s\nNemico: %s\nOggetto: %s\n\n",i+1,tipoZona[zonaAttuale->tipo_zona_mondoreale],tipoNemico[zonaAttuale->nemicoMondoReale->tipoNemico],tipoOggetto[zonaAttuale->tipo_oggetto_mondoreale]);
            i++;
            zonaAttuale=zonaAttuale->avanti;
        }
    }
    printf("----------------------------------------------------\n\n");
}
static void stampa_zona(short type){
     if(mappa_mondoreale==NULL){
        printf("Devi prima generare la mappa\n\n");
        return;
    }
    if(type==0){
        short validInput,posto;
        do{
            printf("Quale zona vorresti vedere [1-%hd]?: ",mapSize);
            validInput=scanf("%hd",&posto);
            pulisciBuffer(validInput);
        }while(validInput!=1);
        posto--;
        if(posto>=0 && posto<mapSize){
            struct zona_mondoreale *zonaAttuale=malloc(sizeof(struct zona_mondoreale));
            zonaAttuale=mappa_mondoreale;
            for(short i=0;i<posto;i++){
                zonaAttuale=zonaAttuale->avanti;
            }
            printf("\n\n-----------------------Zona-%hd----------------------\n",posto+1);
            printf("Mondoreale\n%s\nNemico: %s\nOggetto: %s\n\n",tipoZona[zonaAttuale->tipo_zona_mondoreale],tipoNemico[zonaAttuale->nemicoMondoReale->tipoNemico],tipoOggetto[zonaAttuale->tipo_oggetto_mondoreale]);
            printf("Soprasotto\n%s\nNemico: %s\n\n",tipoZona[zonaAttuale->link_soprasotto->tipo_zona_soprasotto],tipoNemico[zonaAttuale->link_soprasotto->nemicoSoprasotto->tipoNemico]);
        }
        else{
            printf("Posto non esistente\n\n");
        }
        printf("----------------------------------------------------\n\n");
    }
    else{
        if(giocatori[giocatoreAttuale].mondo){
            printf("Soprasotto\n%s\nNemico: %s\n\n",tipoZona[giocatori[giocatoreAttuale].pos_soprasotto->tipo_zona_soprasotto],tipoNemico[giocatori[giocatoreAttuale].pos_soprasotto->nemicoSoprasotto->tipoNemico]);
        }
        else{
            printf("Mondoreale\n%s\nNemico: %s\nOggetto: %s\n\n",tipoZona[giocatori[giocatoreAttuale].pos_mondoreale->tipo_zona_mondoreale],tipoNemico[giocatori[giocatoreAttuale].pos_mondoreale->nemicoMondoReale->tipoNemico],tipoOggetto[giocatori[giocatoreAttuale].pos_mondoreale->tipo_oggetto_mondoreale]);
        }
    }
}
void imposta_gioco(){
    while(giocatori==NULL){
        inserisci_giocatore();
    }
    short scelta,validInput;
    do{
        printf("----------------------Menù-mappa--------------------\n");
        printf("1)Genera mappa\n2)Inserisci zona\n3)Cancella zona\n4)Stampa mappa\n5)Stampa zona\n6)Chiudi mappa\nScegli un opzione del menu:");
        validInput=scanf("%hd",&scelta);
        pulisciBuffer(validInput);
        printf("----------------------------------------------------\n\n");
    }while(validInput!=1);
    switch(scelta){
        case 1:
            genera_mappa();
            break;
        case 2:
            inserisci_zona();
            break;
        case 3:
            cancella_zona();
            break;
        case 4:
            stampa_mappa();
            break;
        case 5:
            stampa_zona(0);
            break;
        case 6:
            return;
            break;
        default:
            printf("Non hai scelto un opzione valida\n\n");
            break;
    }
    imposta_gioco();
}
void gioca(){
    if(giocatori==NULL || mappa_mondoreale==NULL){
        printf("Prima di poter giocare devi impostare il gioco\n\n");
        return;
    }
    short validInput,scelta;
    short turni=0;
    giocatoreAttuale=rand()%quantitaGiocatori;
    short giocatoreIniziale=giocatoreAttuale;
    while(checkDemotorzone() && giocatori!=NULL){
        printf("-----------------------Turno-%hd----------------------\n",turni+1);
        do{
            if(!mosso){
                printf("1)Avanza\n");
            }
            if(giocatori[giocatoreAttuale].pos_mondoreale->indietro!=NULL){
                printf("2)Indietreggia\n");
            }
            if(!mosso){
                printf("3)Cambia mondo\n");
            }
            if(checkNemicoPresente()){
                printf("4)Combatti\n");
            }
            printf("5)Le tue stats\n6)Dove sei\n");
            if(giocatori[giocatoreAttuale].pos_mondoreale->tipo_oggetto_mondoreale && !giocatori[giocatoreAttuale].mondo){
                printf("7)Raccogli oggetto\n");
            }
            printf("8)Usa oggetto\n9)Passa\n%s cosa vuoi fare?: ",giocatori[giocatoreAttuale].nome);
            validInput=scanf("%hd",&scelta);
            printf("\n");
            pulisciBuffer(validInput);
        }while(validInput!=1);
        switch(scelta){
            case 1:
                avanza();
                break;
            case 2:
                indietreggia();
                break;
            case 3:
                cambia_mondo();
                break;
            case 4:
                combatti();
                break;
            case 5:
                stampa_giocatore();
                break;
            case 6:
                stampa_zona(1);
                break;
            case 7:
                if(!giocatori[giocatoreAttuale].mondo && giocatori[giocatoreAttuale].pos_mondoreale->tipo_oggetto_mondoreale){
                    raccogli_oggetto();
                }
                break;
            case 8:
                utilizza_oggetto();
                break;
            case 9:
                turni=passa(turni,giocatoreIniziale);
                break;
            default:
                printf("Non hai scelto un opzione valida\n");
                break;
        }
        printf("----------------------------------------------------\n\n");
        checkGiocatore();
    }
    if(!checkDemotorzone()){
        addScore(turni);
        printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n             X     X    XXX    X     X              \n              X   X    X   X   X     X              \n               X X    X     X  X     X              \n                X     X     X  X     X              \n                X     X     X  X     X              \n                X      X   X    X   X               \n                X       XXX      XXX                \n                                                    \n             X           X  X  X     X              \n             X           X  X  XX    X              \n              X         X   X  X X   X              \n              X    X    X   X  X  X  X              \n               X  X X  X    X  X   X X              \n               X X   X X    X  X    XX              \n                X     X     X  X     X              \nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    }
    if(giocatori==NULL){
        printf("Tutti i giocatori sono morti\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n           XXXX      X     XX   XX  XXXXXXX         \n          X    X    X X    X X X X  X               \n         X          X X    X  X  X  X               \n         X         X   X   X     X  XXXX            \n         X   XXX   XXXXX   X     X  X               \n          X    X  X     X  X     X  X               \n           XXXX   X     X  X     X  XXXXXXX         \n                                                    \n           XXX    X     X  XXXXXXX  XXXXXX          \n          X   X   X     X  X        X     X         \n         X     X   X   X   X        X     X         \n         X     X   X   X   XXXX     XXXXXX          \n         X     X    X X    X        X   X           \n          X   X     X X    X        X    X          \n           XXX       X     XXXXXXX  X     X         \nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    }
    elimina_mappa();
    mosso=0;
}
void termina_gioco(){
    printf("Arrivederci giocatori\n");
    for(short i=0;i<quantitaGiocatori;i++){
        printf("•%s\n",giocatori[i].nome);
    }
    printf("Tornate a trovarci a Occhinz e nel soprasotto\n");
    printf("\033[0m");
    free(giocatori);
    free(scores);
}
void crediti(){
    printf("---------------------Leaderboard--------------------\n");
    printf("Gioco creato da Saponaro Raffaele su un idea di Santini Franceso\nI vincitori delle ultime 3 partite sono:\n");
    for(short i=0;i<3;i++){
        if(scores!=NULL && scores[i].mosse>=1){
            printf("•%s in %hd turni con %hd punti vita rimanenti\n",scores[i].vincitore,scores[i].mosse,scores[i].hp);
        }
        else{
            printf("•Nessun dato\n");
        }
    }
    printf("----------------------------------------------------\n\n");
}