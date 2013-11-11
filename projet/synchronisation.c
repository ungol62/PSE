#include "pse.h"
#include "midi.h"
#include "synchronisation.h"

//Déclaration des variables globales
Buffer midi_data;
pthread_mutex_t mutex_libre = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_plein = PTHREAD_MUTEX_INITIALIZER;
sem_t sCons, sProd;

void production(int midi_in)
{
	int arret = FALSE, indice = -1;
	unsigned char valeur;

	printf("Début de la communication")

  	while (arret == FALSE) 
  	{
	    //Production de la donnée
		retour = midi_RD(midi_in, &valeur); //On lit les données midi reçues en entrée (fichier ou socket)
	  	if (retour == MIDI_EOF || retour == -1) 
	  	{
			erreur("Midi_RD\n");
	    }

	    //Ajout au buffer
	    if ( sem_wait(&sProd) != 0 ) //On attend que de la place se libère si besoin, et on signale un élément produit supplémentaire
	    {
	      	erreur_pthread_IO("sem_wait sProd");
	    }

	    if ( pthread_mutex_lock(&mutex_libre) != 0 ) //On bloque la ressource et on attend si elle est occupée
	    {
	      	erreur_pthread_IO("pthread_mutex_lock libre");
	    }

	    indice = tampon_deposer(&midi_data, valeur); //on dépose la valeur dans le tampon et on récupère l'indice de la case où la valeur a été déposée

	    if ( pthread_mutex_unlock(&mutex_libre) != 0) // On libère la ressource
	    {
	      	erreur_pthread_IO("pthread_mutex_unlock libre");
	    }

	    if ( sem_post(&sCons) != 0) //On signale la production d'une valeur
	    {
	      	erreur_pthread_IO("sem_post sCons");
	    }
	}

  	pthread_exit(NULL);
}

void consommation(void) 
{
  	int arret = FALSE, indice;
  	unsigned char valeur;

  	printf("Démarrage de la consommation")

  	while (arret == FAUX) 
  	{	
  		//Lecture du buffer 
	    if ( sem_wait(&sCons) != 0) //On vérifie qu'il y ait des données à consommer et on les attend si besoin
	    {
	      erreur_pthread_IO("sem_wait sCons");
	    }

	    if ( pthread_mutex_lock(&mutex_plein) != 0)  //On bloque la ressource une fois libérée
	    {
	      erreur_pthread_IO("pthread_mutex_lock plein");
	    }

	    indice = tampon_prendre(&midi_data, &valeur); //On récupère la valeur produite

	    if ( pthread_mutex_unlock(&mutex_plein) != 0 ) //On débloque la ressource
	    {
	      erreur_pthread_IO("pthread_mutex_unlock plein");
	    }

	    if ( sem_post(&sProd) != 0) //On signale la libération d'une place
	    {
	      erreur_pthread_IO("sem_post sProd");
	    }
	    
	    // Récupération des données consommées 
	    //usleep(ATTENTECONS); //On attend un certain temps 
	    retour_clients(&valeur);
	}

  	pthread_exit(NULL);
}

void tampon_init(Buffer *buff) 
{
  int i;
  buff->iLibre = 0;
  buff->iPlein = 0;
  for (i=0; i<BUFF_SIZE; i++) buff->buffer[i] = 0;
}

int tampon_deposer(Buffer *buff, unsigned char valeur) 
{
  int indice;
  indice = buff->iLibre;
  buff->buffer[buff->iLibre] = valeur;
  buff->iLibre = (buff->iLibre + 1) % BUFF_SIZE;
  return indice;
}

int tampon_prendre(Buffer *buff, unsigned char *valeur) 
{
  int indice;
  indice = buff->iPlein;
  *valeur = buff->buffer[buff->iPlein];
  buff->iPlein = (buff->iPlein + 1) % BUFF_SIZE;
  return indice;
}

void retour_clients(unsigned char *valeur)
{
	for (i=0; i<nb_players; i++) //Boucle plutôt sale, qui cherche le premier thread libre dans notre cohorte
    {
    	if (!workers[i].libre)
    	{
    		retour = midi_WR(workers[i].canal, valeur);
    		if(retour != 1)
    		{
    		    erreur("midi_WR\n");
    		}
    	}
    }
}