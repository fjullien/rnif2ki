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
int getToken(char *dest, const char *source);
void testData(void);

int main(int argc, char **argv)
{
	FILE *Source = NULL;
	FILE *Destination = NULL;
	int c,n,p,nombreCompos,nombreNets;
	char buffer[FILELEN];
	time_t t;
	errno_t err;

	if (argc!=3)
	{
		const char *prog = strrchr(argv[0], '\\');

		if (prog == NULL) prog = strrchr(argv[0], '/');
		if (prog == NULL) prog = argv[0]; else prog++;

		printf("\nUsege:\n\t%s input_file output_file\n", prog);
		return -1;
	}

	err = fopen_s (&Destination, argv[2], "w");
	if (err != 0L)
	{
		printf("Couldn't open: %s\n",argv[2]);
		return -1;
	}

	err = fopen_s(&Source, argv[1],"rb");
	if (err != 0L)
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

//#define TEST
#ifdef TEST
	nombreCompos = 2;
	nombreNets = 1;
	testData();
#else
	// recupère le nom des composants/nets et les place dans le tableau
	nombreCompos = getComposNames(buffer, tc);
	nombreNets   = getNetsNames(  buffer, tn);

	// initialisation du tableau des composants

	for (c=0; c<nombreCompos; c++) 
		initComposant(buffer,c,&tc[0]);
#endif
	printf("%d components found\n" "%d nets found\n\n", nombreCompos, nombreNets);

	// Ecriture de la première partie du fichier kicad

    time(&t);

	char buf[80];
	ctime_s(buf, sizeof(buf), &t);
	buf[strlen(buf)-1] = '\0';

	// header
	fprintf(Destination, "(export (version D)\n");
	fprintf(Destination, "  (design\n");
	fprintf(Destination, "    (source \"%s\")\n", argv[2]);
	fprintf(Destination, "    (date \"%s\")\n", buf);
	fprintf(Destination, "    (tool \"rnif2ki v2\")\n");
	fprintf(Destination, "  )\n");

	/*
	(components
	(comp (ref P4)
	(value LOWER_PINS)
    (footprint RMC_pcb_header:pin_header_2.54mm_1x18)
		(fields
		(field (name populate) never))
		(libsource (lib conn) (part CONN_01X18))
	(sheetpath (names /) (tstamps /))
	(tstamp 4E446864))
	...
	)
	*/

	fprintf(Destination, "  (components\n");
	for (c = 0; c < nombreCompos; c++)
	{
		fprintf(Destination, "    (comp (ref %s)\n", tc[c].nom);
		fprintf(Destination, "      (value \"%s\")\n", tc[c].label);
		fprintf(Destination, "      (footprint Capacitors_SMD:C_0603)\n");
		fprintf(Destination, "      (sheetpath (names /) (tstamps /))\n");
		fprintf(Destination, "      (tstamp %X))\n", (unsigned int)t);
	}
	fprintf(Destination, "  )\n");

/*
	(libparts
	(libpart (lib device) (part C)
	(description "Condensateur non polarise")
		(footprints
		(fp SM*)
		(fp C?)
		(fp C1-1))
	(fields
	(field (name Reference) C)
	(field (name Value) C)
	(field (name Footprint) ~)
	(field (name Datasheet) ~))
	(pins
	(pin (num 1) (name ~) (type passive))
	(pin (num 2) (name ~) (type passive))))
	...
	)
*/
#if 0
	fprintf(Destination, "  (libparts\n");
	for (c = 0; c < nombreCompos; c++)
	{
		fprintf(Destination, "    (libpart (lib todo) (part todo)\n");
		fprintf(Destination, "      (description \"todo\")\n");
		fprintf(Destination, "      (fields\n");
		fprintf(Destination, "        (field (name Reference) todo)\n", tc[c].nom);
		fprintf(Destination, "        (field (name Value) todo)\n", tc[c].label);
		fprintf(Destination, "        (field (name Footprint) ~)\n");
		fprintf(Destination, "        (field (name Datasheet) ~))\n");
		fprintf(Destination, "      (pins\n");

		for (p = 0; p <= tc[c].pin_max; p++)
		{
			if (tc[c].pin[p].nom[0])
			{
				if (tc[c].pin[p].alias[0])
					fprintf(Destination, "        (pin(num %s) (name ~) (type passive))\n", tc[c].pin[p].alias);
				else
					fprintf(Destination, "        (pin(num %d) (name ~) (type passive))\n", p);
			}
		}
		fprintf(Destination,"      ))\n");
	}
#endif

	// need libraries ?
	fprintf(Destination, "  (libraries\n");
	fprintf(Destination, "    (library(logical device)\n");
	fprintf(Destination, "    (uri /usr/share/kicad/library/device.lib)))\n");


	// Ecriture de la deuxxième partie du fichier kicad

	fprintf(Destination, "  (nets\n");

	for (n=0; n<nombreNets; n++)
	{
		fprintf(Destination,"    (net (code %d) (name \"%s\")\n", n+1, tn[n].nom);

		for (c=0; c<nombreCompos; c++)
		{
			for (p = 0; p <= tc[c].pin_max; p++)
			{
				if (!strcmp(tc[c].pin[p].nom, tn[n].nom))
				{
					if (tc[c].pin[p].alias[0])
						fprintf(Destination,"      (node (ref %s) (pin %s))\n", tc[c].nom, tc[c].pin[p].alias);
					else
						fprintf(Destination,"      (node (ref %s) (pin %d))\n", tc[c].nom, p);
				}
			}
		}
		fprintf(Destination, "    )\n");
	}

	fprintf(Destination,"))\n");

	fclose(Destination);
	fclose(Source);
	return 0;
}

void testData(void)
{
	tc[0].pin_max = 2;
	strcpy_s(tc[0].nom, NAMELEN, "R");
	strcpy_s(tc[0].label, NAMELEN, "1k");
	strcpy_s(tc[0].pin[0].alias, NAMELEN, "1");
	strcpy_s(tc[0].pin[1].alias, NAMELEN, "2");
	strcpy_s(tc[0].pin[1].nom, NAMELEN, "NET1");

	tc[1].pin_max = 2;
	strcpy_s(tc[1].nom, NAMELEN, "D");
	strcpy_s(tc[1].label, NAMELEN, "1N4148");
	strcpy_s(tc[1].pin[0].alias, NAMELEN, "1");
	strcpy_s(tc[1].pin[0].nom, NAMELEN, "NET1");
	strcpy_s(tc[1].pin[1].alias, NAMELEN, "2");

	strcpy_s(tn[0].nom, NAMELEN, "NET1");
}

void initComposant(char *pointeur,int indexComposant,COMPOSANT *tableComposants)
{
	char compoEnCours[NAMELEN];
	char netEnCours[NAMELEN];
	char temp[NAMELEN];
	int i;
	int pinNonNum = 1;
	int testTemp;

	strcpy_s(compoEnCours, NAMELEN, tableComposants[indexComposant].nom);

	while(pointeur = strstr(pointeur,".ADD_TER"))
	{
		strcpyBetweenChars(netEnCours, pointeur, '"');		// On va recup le nom du signal

		while(*pointeur != '\r' && *pointeur != '\n')
		{
				if (*pointeur == '.')
					while (*pointeur++ != ' ');
				pointeur += 1 + getToken(temp, pointeur);		// On va recup le nom du composant

				if (!strcmp(compoEnCours, temp))
				{
					pointeur += 1 + getToken(temp, pointeur);

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
						if (i < PINSMAX) strcpy_s(tableComposants[indexComposant].pin[i].alias, NAMELEN, temp);
					}
					if (i >= PINSMAX)
						printf("ERROR: %s (%s) contains pin numbers exceeding max.: %d\n", tableComposants[indexComposant].nom, tableComposants[indexComposant].label, PINSMAX-1);
					else
					{
						strcpy_s(tableComposants[indexComposant].pin[i].nom, NAMELEN, netEnCours);
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
	while (*source != Ch && *source != '\n' && len < NAMELEN) 
	{ 
		*dest++ = *source++; 
		len++; 
	}
	*dest = 0;
	return len;
}

int getToken (char *dest, const char *source)
{
	int len = 0;
	while (*source != ' ' && *source != '\r' && *source != '\n' && len < NAMELEN)
	{
		*dest++ = *source++;
		len++;
	}
	*dest = 0;
	return len;
}

