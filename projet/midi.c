#include "pse.h"

//Déclaration des constantes (à intégrer dans un .h)
#define TRUE 1
#define FALSE 0
#define MIDI_EOF 0

//Déclaration des prototypes de fonctions (à intégrer dans in .h)

//Déclaration des variables globales (idem)


//Fonction midi read 
int midi_RD(int midifile, unsigned char *buffer)
{
	int retour;
  	unsigned char car;

  	if (buffer == NULL) //Si le buffer n'existe pas on positionne ERRNO et on renvoie -1
  	{
		errno = EINVAL;
	   	return -1;
	}

    retour = read(midifile, &car, 1);   //on lit l'octet suivant
    if (retour == -1) //si erreur de lecture, retour -1
    {
      return -1;
    }      
    else if (retour == 0) //si EOF
    {
      *buffer = 0x03; //Valeur non affectée dans le protocole MIDI (le \0 correspond à un octet de contrôle qui demande le changement de banque de sons)
      return MIDI_EOF;
    }    
    *buffer = car;    
    return 1;  
}

int midi_WR(int midifile, unsigned char *buffer) 
{
  	int retour;
    retour = write(midifile, buffer, 1);
    if (retour == -1) 
	{
  		return -1;
	}
  	return 1;
}

//Fonction principale de test

/*int main(int argc, char *argv[])
{
	int midiIN, midiOUT, retour, nb = 0;
	char octet;
	midiIN  = open("in.mid", O_RDONLY);
	if (midiIN == 0)
	{
		erreur_IO("open in.mid");
	}
	midiOUT = open("out.mid", O_WRONLY|O_APPEND|O_CREAT);
	if (midiOUT == 0)
	{
		erreur_IO("open OUT.mid");
	}

	while (1)
	{
		retour = midi_RD(midiIN, &octet);
		printf ("%d octets lus \n", nb+1);
		if (retour == MIDI_EOF)
			break;
		retour = midi_WR(midiOUT, &octet);
		nb++;
		printf("%d octets écrits \n", nb);
	}

	close(midiOUT);
	close(midiIN);
	return 0;
}*/