#include "pse.h"

//Décalration des constantes
#define TRUE 1
#define FALSE 0

//Déclaration des variables globales


//Fonction principale
int main(int argc, char const *argv[])
{
	//Déclaration des variables locales
	int sock, arret = FAUX, ret, nbecr;
  	struct sockaddr_in *sa;
  	char texte[LIGNE_MAX];

  	//Connexion au serveur passé en paramètre
  	if (argc != 3) 
  	{
    	erreur("usage: %s machine port\n", argv[0]);
  	}
  	printf("client: creating a socket\n");
	sock = socket (AF_INET, SOCK_STREAM, 0); //Création du socket
	if (sock < 0) 
	{
	  	erreur_IO("socket");
	}
	printf("client: DNS resolving for %s, port %s\n", argv[1], argv[2]);
	sa = resolv(argv[1], argv[2]); //Résolution DNS
	if (sa == NULL) 
	{
	  erreur("address %s and port %s unknown\n", argv[1], argv[2]);
	}
	printf("client: adr %s, port %hu\n", stringIP(ntohl(sa->sin_addr.s_addr)),
	ntohs(sa->sin_port));


	printf("client: connecting the socket\n");
  	ret = connect(sock, (struct sockaddr *) sa, sizeof(struct sockaddr_in)); //connexion au socket créé
  	if (ret < 0) 
  	{
    	erreur_IO("Connect");
  	}

  	freeResolv(); //libération des ressources allouées à la résolution DNS

  	while (arret == FALSE) 
  	{
    	printf("ligne> ");
    	if (fgets(texte, LIGNE_MAX, stdin) == NULL) 
    	{
	      	printf("End of file (or error) :  stopping.\n");
	      	arret = TRUE;
	      	continue; //On sort de la boucle
    	}
    	else 
    	{
	      	nbecr = ecrireLigne(sock, texte); //On peut envoyer des données au serveur (notamment le signal de fin /end)
	      	if (nbecr == -1) 
	      	{
				erreur_IO("ecrireLigne");
     		}
    	  	if (strcmp(texte, "/end\n") == 0) 
    	  	{
				printf("Client. ask to stop.\n");
				arret = VRAI;
    	  	}
    	  	else 
    	  	{
				printf("Client: %d bytes line sent to server.\n", nbecr);
   	   		}
   	 	}
  	}

  	exit(EXIT_SUCCESS);
}