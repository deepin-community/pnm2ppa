/***************************************************************************
    text-nl.h  -  contains the Dutch strings
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 2000 by Jimmy Tak
    email                : fltak@studenten.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* messages from cutswath.c */

/* could not malloc data storage */
/* 00 */ "er kon geen geheugen toegewezen worden voor de opslag van gegevens\n",
  
/* could not read top margin */
/* 01 */ "de bovenste marge kon niet gelezen worden\n",
  
/* could not clear bottom margin */
/* 02 */ "de onderste marge kon niet leeggemaakt worden\n",
  
/* could not read next line */
/* 03 */ "de volgende regel kon niet gelezen worden\n",
  
/* could not malloc ppa storage */
/* 04 */ "er kon geen geheugen toewijzen worden voor de ppa opslag\n",
  
/* error calling */
  /* 05 */ "fout tijdens het aanroepen van ", /*followed by function name*/
  
/* unknown return code in switch */
/* 06 */ "onbekende return code in switch\n",
  
  /* from ppa.c */

/* truncating nozzle data to 6 rows, (was */
  /* 07 */ "spuitgat gegevens worden afgekapt tot 6 rijen, (het waren ", /* followed by number*/
  
  /* messages from image.c */
  
/* Could not create gamma file. Check permissions */
/* 08 */ "gamma.ppm bestand kon niet worden gemaakt. Controleer de toegangsrechten\n",
  
/* Truncated input - EOF while reading image: */
  /* 09 */ "Afgekapte invoer - EOF tijdens het lezen van de afbeelding: ", /*followed by format*/
  
/* Fatal error - failed to read input image */
/* 10 */ "Fatale fout - invoer afbeelding kon niet gelezen worden\n",
  
/* Fatal error - input image has pixel with value > 255 */
/* 11 */ "Fatale fout - invoer afbeelding heeft een pixel met een waarde groter dan 255\n",
  
/* Fatal error! Unable to allocate buffer memory! */
/* 12 */ "Fatale fout! Kan geen buffer geheugen toewijzen!\n",
  
/* Input image is not a supported PNM format */
/* 13 */ "Fatale fout! Invoer afbeelding (pnm2ppa) is niet een PNM indeling\n",
  
/* Unknown PNM magic */
  /* 14 */  /*removed*/

  
/* Printer cannot print on paper size */
  /* 15 */ "De printer kan niet afdrukken op de papiergrootte ", /*followed by size */
  
/* maximum PNM pixel value must be 255, but is: */
  /* 16 */ "maximale PNM pixel waarde moet 255 zijn, maar het is: ", /*followed by value*/
  
  /* messages from ppa.c and pnm2ppa.c */
  
/* Unknown Printer version */
/* 17 */ "Onbekende printer versie\n",
  
/* Could not malloc storage for compressed data */
/* 18 */ "Er kon geen geheugen toegewezen voor de opslag van gecomprimeerde gegevens\n",
  
/* Starting print job */
/* 19 */ "Afdruk opdracht wordt gestart\n",
  
/* Printing page */
  /* 20 */ "Bezig met afdrukken van pagina ", /*followed by page number */
  
/* Finished rendering page */
  /* 21 */ "Klaar met afbeelden van pagina ",  /*followed by page number */
  
/* No pages printed! */
/* 22 */ "Geen pagina's afgedrukt!\n",
  
/* Print job completed successfully. */
/* 23 */  "Afdruk opdracht is succesvol afgerond.\n",
  
/* Verbose logging to console */
/* 24 */ "Uitgebreide logweergave in de console\n",
  
/* Please configure your printer in /etc/pnm2ppa.conf */
/* 25 */ "Configureer aub uw printer in /etc/pnm2ppa.conf\n",
  
/* couldn't open config file */
/* 26 */ "configuratie bestand kon niet geopend worden\n", 

/* unknown paper size */
/* 27 */ "onbekende papiergrootte\n", 

/* unrecognized parameter */
  /* 28 */ "onbekende parameter", /*not followed by \n */ 

/* at line */
  /* 29 */ "in rij ", /* followed by line number */ 

/* error parsing config file */
  /* 30 */  "fout bij het interpreteren van het configuratie bestand", /* not followed  by \n */

/* Fatal: unknown argument */  
/* 31 */ "Fataal: onbekend argument\n",  

/* Could not open PPA output file */
/* 32 */ "PPA uitvoer bestand kon niet geopend worden\n", 
  
/* Could not open PNM input file */
/* 33 */ "PNM invoer bestand kon niet geopend worden\n", 

/* version and build date: */
  /* 34 */ "versie en compileer datum: ", /*followed by version number and date*/

  /* 35 */ "Color correction curve (Gamma) file exists, but is corrupt.\n",


  /* 36 */ "POSSIBLE BUFFER OVERFLOW ATTACK!!: length of a path specified as an option exceeds", /*followed by maximum allowed size*/

  /* FATAL: Bad error message */

  "FATAAL: Slechte foutmelding\n"
