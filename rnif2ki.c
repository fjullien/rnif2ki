/*
* (C) Copyright 2008, Franck Jullien, <franck.jullien@gmail.com>
*
* See file CREDITS for list of people who contributed to this
* project.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILELEN 100000
#define NAMELEN 64
#define PINSMAX 512
#define NETSMAX 1024
#define COMPMAX 1024

typedef struct _pin {
	char nom[NAMELEN];
	char alias[NAMELEN];
}PIN;

typedef struct _composant {
	char nom[NAMELEN];
	char label[NAMELEN];
	int pin_max;
	PIN pin[PINSMAX];
}COMPOSANT;


typedef struct _nets {
	char nom[NAMELEN];
}NETS;

COMPOSANT tc[COMPMAX];
NETS tn[NETSMAX];

int getNetsNames(char *pointeur,NETS *tableNets);
int getComposNames(char *pointeur,COMPOSANT *tableComposants);
void initComposant(char *pointeur,int indexComposant,COMPOSANT *tableComposants);
int strcpyBetweenChars(char *dest, const char *source, char Ch);

int main(int argc, char **argv)
{
	FILE *Source = NULL;
	FILE *Destination = NULL;
	int c,n,p,nombreCompos,nombreNets;
	char buffer[FILELEN];
	time_t t;

	if (argc!=3)
	{
		const char *prog = strrchr(argv[0], '\\');

		if (prog == NULL) prog = strrchr(argv[0], '/');
		if (prog == NULL) prog = argv[0]; else prog++;

		printf("\nUsege:\n\t%s input_file output_file\n", prog);
		return -1;
	}

	Destination = fopen(argv[2],"w");
	if (Destination==NULL)
	{
		printf("Couldn't open: %s\n",argv[2]);
		return -1;
	}

	Source = fopen(argv[1],"rb");
	if (Source==NULL)
	{
		printf("Couldn't open: %s\n",argv[1]);
		fclose(Destination);
		return -1;
	}

	fread(buffer, 1, sizeof(buffer), Source);
	if (ferror(Source))
	{
		printf("File: %s read error\n",argv[1]);
		fclose(Destination);
		fclose(Source);
		return -1;
	}

	printf("\nProgram limits (recompile to change):\n\n"
		   "\tInput file length       (max. bytes):\t%d\n"
		   "\tComponent/net/pin name (max. length):\t%d\n"
		   "\tNumber of nets                (max.):\t%d\n"
		   "\tNumber of components          (max.):\t%d\n"
		   "\tPins per component            (max.):\t%d\n\n", FILELEN, NAMELEN, NETSMAX, COMPMAX, PINSMAX);

	// recupère le nom des composants/nets et les place dans le tableau
	nombreCompos = getComposNames(buffer, tc);
	nombreNets   = getNetsNames(  buffer, tn);

	printf("%d components found\n" "%d nets found\n\n", nombreCompos, nombreNets);

	// initialisation du tableau des composants

	for (c=0; c<nombreCompos; c++) initComposant(buffer,c,&tc[0]);

	// Ecriture de la première partie du fichier kicad

    time(&t);

	fprintf(Destination,"# EESchema Netlist Version 1.1 created  %s\n", ctime(&t));
	fprintf(Destination,"(\n");

	for (c=0; c<nombreCompos; c++)
	{
		fprintf(Destination," ( %03d $noname %s %s\n",c+1, tc[c].nom, tc[c].label);
		for (p = 0; p <= tc[c].pin_max; p++)
		{
			if (tc[c].pin[p].nom[0])
			{
				if (tc[c].pin[p].alias[0])
					fprintf(Destination,"  (    %s %s )\n", tc[c].pin[p].alias, tc[c].pin[p].nom);
				else
					fprintf(Destination,"  (    %d %s )\n", p, tc[c].pin[p].nom);
			}
		}
		fprintf(Destination," )\n");
	}

	fprintf(Destination, ")\n");

	// Ecriture de la deuxxième partie du fichier kicad

	fprintf(Destination, "*\n" "{ Pin List by Nets\n");

	for (n=0; n<nombreNets; n++)
	{
		fprintf(Destination,"Net %d \"%s\"\n", n, tn[n].nom);

		for (c=0; c<nombreCompos; c++)
		{
			for (p = 0; p <= tc[c].pin_max; p++)
			{
				if (!strcmp(tc[c].pin[p].nom, tn[n].nom))
				{
					if (tc[c].pin[p].alias[0])
						fprintf(Destination," %s %s\n",tc[c].nom, tc[c].pin[p].alias);
					else
						fprintf(Destination," %s %d\n",tc[c].nom, p);
				}
			}
		}
	}

	fprintf(Destination,"}\n");
	fprintf(Destination,"#END\n");

	fclose(Destination);
	fclose(Source);
	return 0;
}

void initComposant(char *pointeur,int indexComposant,COMPOSANT *tableComposants)
{
	char compoEnCours[NAMELEN];
	char netEnCours[NAMELEN];
	char temp[NAMELEN];
	int i;
	int pinNonNum = 1;
	int testTemp;

	strcpy(compoEnCours,tableComposants[indexComposant].nom);

	while(pointeur = strstr(pointeur,".ADD_TER"))
	{
		strcpyBetweenChars(netEnCours, pointeur, '"');		// On va recup le nom du signal

		while(*pointeur != '\r' && *pointeur != '\n')
		{
				pointeur += 9;
				pointeur += 1 + strcpyBetweenChars(temp, pointeur, ' ');		// On va recup le nom du composant

				if (!strcmp(compoEnCours, temp))
				{
					pointeur += 1 + strcpyBetweenChars(temp, pointeur, ' ');

					for (i = 0, testTemp = 0; temp[testTemp]; testTemp++ )
					{
						if (temp[testTemp] >= '0' && temp[testTemp] <= '9')
							i = i * 10 + (temp[testTemp] - '0');
						else
						{
							i = -1;
							break;
						}
					}

					if (i == -1)
					{
						printf("Warning: %s (%s), Pin: %s (net: %s) does not use pin number.\n", tableComposants[indexComposant].nom, tableComposants[indexComposant].label, temp, netEnCours);
						i = pinNonNum++;
						if (i < PINSMAX) strcpy(tableComposants[indexComposant].pin[i].alias, temp);
					}
					if (i >= PINSMAX)
						printf("ERROR: %s (%s) contains pin numbers exceeding max.: %d\n", tableComposants[indexComposant].nom, tableComposants[indexComposant].label, PINSMAX-1);
					else
					{
						strcpy(tableComposants[indexComposant].pin[i].nom, netEnCours);
						if (tableComposants[indexComposant].pin_max < i) tableComposants[indexComposant].pin_max = i;
					}
				}

				while(*pointeur++ != '\n');
		}
	}
}

int getComposNames(char *pointeur,COMPOSANT *tableComposants)
{
	int composantsTrouves;

	for (composantsTrouves = 0; composantsTrouves < COMPMAX && (pointeur = strstr(pointeur,".ADD_COM")); composantsTrouves++)
	{
		pointeur += 1 + strcpyBetweenChars(tableComposants[composantsTrouves].nom,   pointeur, ' ');
		pointeur += 1 + strcpyBetweenChars(tableComposants[composantsTrouves].label, pointeur, '"');
	}

	return composantsTrouves;
}

int getNetsNames(char *pointeur,NETS *tableNets)
{
	int netsTrouves;

	for (netsTrouves = 0; netsTrouves < NETSMAX && (pointeur = strstr(pointeur,".ADD_TER")); netsTrouves++)
	{
		pointeur += 1 + strcpyBetweenChars(tableNets[netsTrouves].nom, pointeur, '"');		// On va recup le nom du signal
	}

	return netsTrouves;
}

int strcpyBetweenChars(char *dest, const char *source, char Ch)
{
	int len = 0;
	while (*source++ != Ch) ;
	while (*source   != Ch && *source != '\n' && len < NAMELEN) { *dest++ = *source++; len++; }
	*dest = 0;
	return len;
}
