#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checksum.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/wait.h>

#define MSG_ERR_B "Seul l'option -b est supportée\n"
#define MSG_ERR_TAILLE "negatif ou nul >0\n"
#define MSG_ERR_USAGE "Usage: lcp [-j nb_proc] [-l] [-b TAILLE] SOURCES... DESTINATION\n"
#define MSG_ERR_IMPAIR "pair\n"
#define MSG_ERR_NBR_PROCESSUS "Nombre processus doit être supperieur ou égal à 2\n"
#define MSG_ERR_TYPE "La/Les source(s) doit être un fichier, lien ou un répertoire\n"
#define MSG_ERR_MEM "Allocation de mémoire avec malloc impossible: Manque d'espace\n"

/**
Programme permettant de copier un fichier source vers un fichier ou répertoire destination.
Si le fichier destination existe déja, le programme se charge de faire une copie paresseuse du document source.

Réalisé par :
-EID Luigi (EIDL75070000)
-NAYERI POOR Ali (NAYA18019909)
**/

/***
 * Vérifie si le fichier est un lien symbolique
 * Retourne 1 si c'est le cas.
 * Sinon, Retourne 0
 */
int estLienSym(const char *nom)
{
	struct stat destination;
	lstat(nom, &destination);
	printf("S_ISLNK: %i\n", S_ISLNK(destination.st_mode));
	if (S_ISLNK(destination.st_mode) == 1)
	{
		return 1;
	}
	return 0;
}

/**
Verifier si le nom entré en paramètre est un fichier, un lien, ou un répertoire.
Retourne 1 si c'est l'un des 3.
Retourne 0 sinon.
**/
int estQuelconque(const char *nom)
{
	struct stat destination;
	if (estLienSym(nom) == 1)
	{
		return 1;
	}
	if (stat(nom, &destination) == -1)
	{
		// perror("stat");
		return 0;
	}
	else if (S_ISREG(destination.st_mode) == 1 || S_ISDIR(destination.st_mode) == 1)
	{
		return 1;
	}
	return 0;
}

/**
 * Vérifie si l'argument lu est le dernier argument de la liste ou bien si l'on a lu
 * une option malgré le fait qu'on a un "--"
 * Retourne 1 s'il y a une erreur
 * Retourne 0 si tout est valide
 */
int verifDernierArg(int i, int argc, int finOptions, int considererOption)
{
	if (((i + 1) >= (argc - 1)) || (considererOption == 1 && finOptions == 1))
	{
		// Cela veut dire que c'est le dernier argument et/ou le prochain argument est le dernier ce qui est invalide.
		fprintf(stderr, "%s", MSG_ERR_USAGE);
		return 1;
	}
	return 0;
}

/**
Verifier si les argurments entrés par l'utilisateur sont corrects.
Retourne 1 si erreur 0 si le fonctionnement est correct.
**/
int verifArg(int *taille, int *estLien, int *nbrProcessus, int *indiceFichier, char **argv, int argc)
{
	int finOptions = 0;
	for (int i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], "-b") == 0))
		{
			if (verifDernierArg(i, argc, finOptions, 1) == 1)
			{
				return 1;
			}
			*taille = atoi(argv[++i]);
			if (*taille <= 0)
			{
				fprintf(stderr, "%s", MSG_ERR_TAILLE);
				return 1;
			}
			else if (*taille % 2 != 0)
			{
				fprintf(stderr, "%s", MSG_ERR_IMPAIR);
				return 1;
			}
		}
		else if ((strcmp(argv[i], "-j") == 0))
		{
			if (verifDernierArg(i, argc, finOptions, 1) == 1)
			{
				return 1;
			}
			*nbrProcessus = atoi(argv[++i]);
			if (*nbrProcessus < 2)
			{
				fprintf(stderr, "%s", MSG_ERR_NBR_PROCESSUS);
				return 1;
			}
		}
		else if ((strcmp(argv[i], "-l") == 0))
		{
			*estLien = 1;
			if (verifDernierArg(i, argc, finOptions, 1) == 1)
			{
				return 1;
			}
		}
		else if (((strcmp(argv[i], "--") == 0)))
		{
			if (verifDernierArg(i, argc, finOptions, 1) == 1)
			{
				return 1;
			}
			finOptions = 1;
		}
		else
		{
			if (*indiceFichier == 0)
			{
				if (estQuelconque(argv[i]) != 1)
				{
					fprintf(stderr, "%s", MSG_ERR_TYPE);
					return 1;
				}
				if (verifDernierArg((i - 1), argc, finOptions, 0) != 1)
				{
					*indiceFichier = i;
					finOptions = 1;
				}
				else
				{
					fprintf(stderr, "%s", MSG_ERR_USAGE);
					return 1;
				}
			}
			else
			{
				if ((estQuelconque(argv[i]) != 1) && (i != argc - 1))
				{
					fprintf(stderr, "%s", MSG_ERR_TYPE);
					return 1;
				}
			}
		}
	}
	if (*indiceFichier == 0)
	{
		fprintf(stderr, "%s", MSG_ERR_USAGE);
		return 1;
	}
	return 0;
}
/**
Verifier si le nom entré en paramètre est un répertoire ou non.
Retourne 1 s'il s'agit d'un répertoire.
Retourne 0 si c'est autre chose
**/
int estRepertoire(const char *nom)
{
	struct stat destination;
	if (stat(nom, &destination) == -1)
	{
		perror("stat");
		return 0;
	}
	else
		return S_ISDIR(destination.st_mode);
}
/**
Calcule et retourne la taille d'un fichier.
Retourne la taille du fichier(nombre de caractères).
**/
int tailleFichier(const char *nom, struct stat sb)
{
	if (stat(nom, &sb) == -1)
	{
		perror("stat");
		exit(1);
	}
	else
		return ((sb.st_size));
}

/**
 * Crée un lien symbolique qui pointe au même endroit que notre source avec le nom
 * destination passé en paramètre
 */
void creerSymLink(char *source, char *desti)
{
	struct stat sb;
	char *buffer;
	ssize_t nbytes, buffer_size;
	if (lstat(source, &sb) == -1)
	{
		perror("lstat");
		return;
	}
	buffer_size = sb.st_size + 1;
	if (sb.st_size == 0)
	{
		buffer_size = _PC_PATH_MAX;
	}
	buffer = malloc(buffer_size);
	if (buffer == NULL)
	{
		fprintf(stderr, "%s", MSG_ERR_MEM);
		return;
	}
	nbytes = readlink(source, buffer, buffer_size);
	if (nbytes == -1)
	{
		perror("readlink");
		return;
	}
	symlink(buffer, desti);
	free(buffer);
}

/**
Créer un fichier copie de la source
**/
void creerCopie(const char *nom, int fsource, int fdesti, int taille)
{
	int curPos = 0;
	char buffer[taille];
	fdesti = open(nom, O_RDWR | O_CREAT, 0644);
	while ((curPos = read(fsource, buffer, taille)) != 0)
	{
		write(fdesti, buffer, curPos);
	}
}

/**
Tronque le contenu du fichier 'nom' et copie le contenue du fichier source.
**/
void tronqueCopie(const char *nom, int fsource, int fdesti, int taille)
{
	int curPos = 0;
	char buffer[taille];
	fdesti = open(nom, O_RDWR | O_TRUNC, 0644);
	while ((curPos = read(fsource, buffer, taille)) != 0)
	{
		write(fdesti, buffer, curPos);
	}
}

/**
Effectue une copie parresseuse du fichier source sur le fichier 'nom'.
**/
void copieParresseuse(const char *nom, int taille, int tailleSource, int fsource)
{
	int byteSource, byteDesti;
	int estFin = 0, curPos = 0, fdesti;
	char bufSource[taille];
	char bufDesti[taille];
	uint32_t byte1, byte2;

	fdesti = open(nom, O_RDWR | O_CREAT, 0644);

	while (estFin != 1)
	{
		if ((curPos + taille) > tailleSource)
		{
			taille = (tailleSource - curPos);
		}
		byteSource = read(fsource, bufSource, taille);
		byteDesti = read(fdesti, bufDesti, taille);
		byte1 = fletcher32((uint16_t *)bufSource, taille);
		byte2 = fletcher32((uint16_t *)bufDesti, taille);
		if (byte1 != byte2)
		{
			lseek(fdesti, -byteDesti, SEEK_CUR);
			write(fdesti, bufSource, byteSource);
		}
		curPos = curPos + taille;
		if (curPos >= tailleSource)
		{
			estFin = 1;
		}
	}
}

/**
 * Copie le fichier d'une manière différente selon le scénario.
 */
void copierFichier(char *source, char *desti, int taille, int estLien)
{
	int tailleSource, tailleDesti, fdesti = 0, fsource;
	char *nomSource;
	struct stat sb, sb2;
	if (estLien == 1 && estLienSym(source) == 1)
	{
		if (estRepertoire(desti) == 1)
		{
			char *cheminFichier = strcat(desti, "/");
			nomSource = basename(source);
			cheminFichier = strcat(desti, nomSource);
			creerSymLink(source, cheminFichier);
		}
		else
		{
			creerSymLink(source, desti);
		}
		return;
	}
	else
	{
		// Verification du fichier source
		fsource = open(source, O_RDWR);
		tailleSource = tailleFichier(source, sb);
	}
	if (fsource == -1)
	{
		perror("Source");
	}
	nomSource = basename(source);
	// Si la destination n'existe pas
	if (stat(desti, &sb2) == -1)
	{
		creerCopie(desti, fsource, fdesti, taille);
	}
	// Si la destination est un répertoire
	else if (estRepertoire(desti) == 1)
	{
		char *cheminFichier = strcat(desti, "/");
		cheminFichier = strcat(desti, nomSource);
		creerCopie(cheminFichier, fsource, fdesti, taille);
	}
	// Si la destination est un fichier
	else
	{
		tailleDesti = tailleFichier(desti, sb2);
		if (tailleDesti > tailleSource)
		{
			tronqueCopie(desti, fsource, fdesti, taille);
		}
		else
		{
			copieParresseuse(desti, taille, tailleSource, fsource);
		}
	}
}

int main(int argc, char *argv[])
{
	int taille = 32, estLien = 0, nbrProcessus = 2, indiceFichier = 0;

	// Verification des arguments
	if (argc < 3)
	{
		fprintf(stderr, "%s", MSG_ERR_USAGE);
		exit(1);
	}
	if (verifArg(&taille, &estLien, &nbrProcessus, &indiceFichier, argv, argc) == 1)
	{
		exit(1);
	}

	int nbFichierCP = (argc - 1 - indiceFichier);
	char *listeFichiers[nbFichierCP];
	for (int i = 0; i < nbFichierCP; i++)
	{
		listeFichiers[i] = argv[indiceFichier + i];
	}
	// Assigne le bon nombre de processus
	if (nbFichierCP < nbrProcessus - 1)
		nbrProcessus = nbFichierCP + 1;
	// Variables
	int pidMere = getpid(), pidFils;
	int pidsProcessus[nbrProcessus - 1];
	int i = 0;

	int nbProcessusFils = nbrProcessus - 1;

	while (nbFichierCP != 0)
	{
		if (nbFichierCP < nbProcessusFils)
		{
			nbProcessusFils = nbFichierCP;
		}
		for (int j = 0; j < nbProcessusFils; j++)
		{
			if (getpid() == pidMere)
			{
				pidFils = fork();
			}
			if (pidFils == -1)
			{
				perror("Erreur fork");
			}
			else
			{
				pidsProcessus[i] = pidFils;
			}

			if (pidFils == 0)
			{
				int indice = i + j;
				copierFichier(listeFichiers[indice], argv[argc - 1], taille, estLien);
				exit(0);
			}
		}

		if (getpid() == pidMere)
		{
			for (int i = 0; i < nbProcessusFils; i++)
			{
				wait(&pidsProcessus[i]);
			}
		}

		nbFichierCP = nbFichierCP - nbProcessusFils;
		i = i + nbProcessusFils;
	}

	return 0;
}
