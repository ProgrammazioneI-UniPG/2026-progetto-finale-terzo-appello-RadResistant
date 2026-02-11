typedef enum{
    bosco=0,
    scuola=1,
    laboratorio=2,
    caverna=3,
    strada=4,
    giardino=5,
    supermercato=6,
    centrale_elettrica=7,
    deposito_abbandonato=8,
    stazione_polizia=9
}tipo_zona;
typedef enum{
    billi=0,
    nessun_nemico=1,
    democane=2,
    demotozone=3
} tipo_nemico;
typedef enum{
    nessun_oggetto=0,
    bicicletta=1, // da la possibilita di scappare dalla lotta
    maglietta_fuocoinferno=2, // Aumenta gli hp di 5
    bussola=3, // se usata nel mondo reale indica la direzione del demotorzone
    schitarrata_metallica=4 // aumenta tuttle le stats
}tipo_oggetto;
void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();
struct nemico{
    tipo_nemico tipoNemico;
    short tramortito;
    short puntiVita;
    short attacco;
    short difesa;
};
struct zona_mondoreale{
    tipo_zona tipo_zona_mondoreale;
    tipo_oggetto tipo_oggetto_mondoreale;
    struct nemico *nemicoMondoReale;
    struct zona_mondoreale *avanti;
    struct zona_mondoreale *indietro;
    struct zona_soprasotto *link_soprasotto;
};
struct zona_soprasotto{
    tipo_zona tipo_zona_soprasotto;
    struct nemico *nemicoSoprasotto;
    struct zona_soprasotto *avanti;
    struct zona_soprasotto *indietro;
    struct zona_mondoreale *link_mondoreale;
};
struct giocatore{
    char nome[63];
    enum {mondoreale=0,soprasotto=1} mondo;
    struct zona_mondoreale *pos_mondoreale;
    struct zona_soprasotto *pos_soprasotto;
    short puntiVita;
    short attacco_psichico;
    short difesa_psichica;
    short fortuna;
    tipo_oggetto zaino[3];
};