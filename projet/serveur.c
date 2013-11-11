#include "pse.h"
#include "synchronisation.h"
//Déclaration des constantes
#define TRUE 1
#define FALSE 0
#define NB_ARG 2
#define MAX_PLAYERS 10

//Déclaration des prototypes de fonctions
void init_workers (int nb_threads);
void* musician_management(void* arg);
void listen_socket(int port, int nb_players);

//Déclaration des variables globales
DataSpec workers[MAX_PLAYERS]; //Tableau de structures contenant des données spécifiques à nos workers (voir module dataspec.h)
pthread_t *consommateur;
sem_t sem_libres; //Sémaphores libres
int journal; //Identifier du journal

//Fonction principale
int main(int argc, char const *argv[])
{
	//Déclaration des variables
	int nb_players = 0; //Nombre de musiciens
	int mode, port; //Mode d'ouverture du journal (identifier du journal commun à tous --> variable globale) et numéro de port

	//Vérification des arguments
	if (argc < NB_ARG)
	{
		erreur_IO("usage : port nb_players\n");
	}
	else //on affecte les valeurs des arguments à des variables au nom plus évocateur
	{
		nb_players = atoi(argv[2]); //Récupération du nombre de musiciens
		port = atoi(argv[1]); //Récupération du numéro de port pour la connexion principale
	}

	//Ouverture ou création du journal
	mode = O_WRONLY|O_APPEND|O_CREAT;
	journal = open("journal.log", mode, 0660);
	if (journal == -1) 
	{
		erreur_IO("open journal");
	}
	 
	//Gestion des connexions
	if(nb_players<=0 || nb_players > MAX_PLAYERS) //On vérifie que le nombre de musiciens est correct
	{
		erreur_IO("\nwrong number of players\n");
	}

	if (sem_init(&sem_libres, 0, nb_players) == -1) //On initialise le sémaphore qui compte les threads libres. 
	{
   		erreur_IO("sem_init libres");
  	}
	
	init_workers(nb_players); //On crée notre pool de workers qu'on met en attente de connexion ensuite

	init_session(); //On initialise la session, en créant un thread consommateur qui redirige ensuite les données vers tous les clients (lorsque ceux-ci sont connectés)

	listen_socket(port, nb_players);//On écoute le socket entrant pour récupérer les connexions et les affecter aux threads

	return 0;
}

void init_workers (int nb_threads)
{
	int i = 0; //Variable d'incrémentation pour la boucle for
	for (i = 0; i < nb_threads; i++)
	{
	    workers[i].tid = i; //on donne à chaque thread un identifiant logique correspondant à son indice dans le tableau
	    workers[i].libre = TRUE; //Pour le moment tous les threads sont libres
	    workers[i].canal = -1; //-1 indique que le thread n'a aucune requête à traiter
	    if (sem_init(&workers[i].sem, 0, 0) == -1)  //On initialise le sémaphore de réveil à 0 (thread virtuellement bloqué)
	    {
	      erreur_IO("sem_init");
    	}
	    int ret = pthread_create(&workers[i].id, NULL, musician_management, &workers[i]); //On crée les threads, qu'on assigne à la fonction en argument et on passe la structure en arg, pour pouvoir modifier les champs.
	    if (ret != 0)
	    {
	      erreur_IO("pthread_create");
    	}
  	}
}

void* musician_management(void* arg)
{
	//Déclarations des variables
	DataSpec * data = (DataSpec *) arg; //On caste le pointeur passé en argument pour retrouver un DataSpec (au lieu d'un void*)
  	int arret = FALSE, nblus; //On initialise le booléen qui arrêtera la boucle
  	char texte[LIGNE_MAX]; 

  	//Gestion des musiciens
  	printf("worker %d: waiting for channel.\n", data->tid);
    while(TRUE)
    {
    	if (sem_wait(&data->sem) != 0) //Les semaphores associés aux workers sont initialisés à 0, donc bloqués par la fonction sem_wait, en l'attente d'un passage à 1 (c-à-d de l'affectation du thread) 
	    {
	      erreur_pthread_IO("sem_wait");
	    }
	    data->libre = FALSE; //Une fois l'attente terminée, on marque le thread comme occupé
	    printf("worker %d: reading channel %d.\n", data->tid, data->canal); 

	    arret = FALSE;

	    while (arret == FALSE) //Boucle d'exécution du thread
	    {
	    	nblus = lireLigne (data->canal, texte); //On lit les commandes de l'utilisateur
	      	if (nblus <= 0 || nblus == LIGNE_MAX) 
	      	{
				erreur("lireLigne\n");
		    }

			if (strcmp(texte, "/end") == 0) 
			{
				printf("worker %d: ask for disconnection.\n", data->tid);
				arret = TRUE;
				continue;//On reboucle, et arret = TRUE donc on ne repasse pas par le while, on passe à la suite
		    }
		}
		if (close(data->canal) == -1) //On ferme le canal
		{
	     	erreur_IO("close");
	    }
	    data->canal = -1; //On réinitialise les paramètres
	    data->libre = TRUE; //On indique que le thread est dispo
	    if (sem_post(&sem_libres) != 0) //On marque le sémaphore
	    {
	      	erreur_pthread_IO("sem_post");
	    }
    }//En rebouclant, on attend une nouvelle connexion
    
  	pthread_exit(NULL); //On libère le thread
}


void listen_socket(int port, int nb_players)
{
	//Déclaration des variables
	struct sockaddr_in adrEcoute, reception; //Structures contenant les infos du socket d'ecoute (adrEcoute pour écouter, réception pour affecter ailleurs une fois la connexion reçue)
	socklen_t receptionlen = sizeof(reception);
	int canal, ret, ecoute, ilibre;

	// Création du socket d'écoute
	printf("server: creating a socket\n");
	ecoute = socket (AF_INET, SOCK_STREAM, 0); //Création d'un socket (renvoie un descripteur de fichier) AF_INET = protocole IPv4 et SOCK_STREAM = échange en flux (= fifo mais à double sens) et dernier paramètre toujours à 0 pour AF_INET
	if (ecoute < 0) 
	{
		erreur_IO("socket");
	}
	

	adrEcoute.sin_family = AF_INET; //Protocole
	adrEcoute.sin_addr.s_addr = INADDR_ANY; //Adresse attendue
	adrEcoute.sin_port = htons(port); //Port d'écoute 
	printf("server: binding to INADDR_ANY address on port %d\n", port);
	ret = bind (ecoute,  (struct sockaddr *) &adrEcoute, sizeof(adrEcoute)); //On lie les paramètres d'écoute au socket créé
	if (ret < 0) 
	{
	  erreur_IO("bind");
	}
	
	printf("server: listening to socket\n");
	ret = listen (ecoute, 20); //On écoute le socket en attendant une connexion (bloquant)
	if (ret < 0) 
	{
	  erreur_IO("listen");
	}

	while (TRUE) 
	{
	    printf("server: waiting for a connection\n");
	    canal = accept(ecoute, (struct sockaddr *) &reception, &receptionlen); //Quand on a une connexion, on la récupère et on l'affecte à un canal
	    if (canal < 0) 
	    {
	      erreur_IO("accept");
	    }
	    printf("server: adr %s, port %hu\n", stringIP(ntohl(reception.sin_addr.s_addr)), ntohs(reception.sin_port));

	    if (sem_wait(&sem_libres) == -1) //On attend qu'un thread se libère
	    {
	      erreur_IO("sem_post");
	    }
	    for (ilibre=0; ilibre<nb_players; ilibre++) //Boucle plutôt sale, qui cherche le premier thread libre dans notre cohorte
	    {
	    	if (workers[ilibre].libre)
	    		break;	
	    }
	      
	    printf("serveur: %d\n", ilibre);

	    workers[ilibre].canal = canal; //on affecte le canal récupéré plus haut à notre worker
	    if (sem_post(&workers[ilibre].sem) == -1) //On marque le sémaphore
	    {
	      erreur_IO("sem_post");
	    }
	    printf("server: worker %d chosen\n", ilibre);   
    }
}

void init_session()
{
	int retour = pthread_create(consommateur, NULL, consommation);
	if (ret !=0)
	{
		erreur_IO("pthread_create consommateur");
	}
}