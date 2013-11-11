#ifndef TAMPON_INCLUDE_H
#define TAMPON_INCLUDE_H

//Déclaration des constantes
#define TRUE 1
#define FALSE 0
#define BUFF_SIZE 20


/* Type Buffer à BUFF_SIZE cases */
typedef struct buffer_t  {
  int iLibre;       /* indice de la premiere case libre */
  int iPlein;       /* indice de la premiere case occupee */
  unsigned char buffer[BUFF_SIZE];    /* buffer contenant les valeurs produites et consommees */
} Buffer;

/*Produit les données a mettre dans le buffer à partir
des données récupérées dans le socket, ici midi_in*/
void production(int midi_in);

/*Consomme les données produites dans le buffer et les renvoie vers la
sortie midi_out*/
void consommation(int midi_out);

/* Renvoie la valeur consommée aux clients connectés */
void retour_clients(unsigned char *valeur);

/* Initialisation du buffer buff.
   Tous les champs sont initialises à 0 */
void tampon_init(Buffer *buff);

/* Depot d'une valeur valeur dans le tampon buff.
   Le champ iLibre est utilise pour stocker valeur.
   L'indice de la case où est déposée la valeur est retourné. */
int tampon_deposer(Buffer *buff, unsigned char valeur) ;

/* Recuperation d'une valeur valeur depuis le tampon buff.
   Le champ iPlein est utilisé pour récupérer valeur.
   L'indice de la case ou était stockée la valeur valeur est retourné. */
int tampon_prendre(Buffer *buff, unsigned char *valeur);

#endif

