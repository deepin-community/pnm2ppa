/***************************************************************************
    text-it.h  -  contains the Italian  strings
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 2000 by Andrew van der Stock
    email                : ajv@greebo.net

    Translated to Italian by Riccardo Murri <murri@phc.unipi.it>, Fri Aug 11 18:34:09 CEST 2000
    Tradotto in Italiano da Riccardo Murri <murri@phc.unipi.it>, Fri Aug 11 18:34:09 CEST 2000

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

/* 00 */ "impossibile allocare memoria per i dati\n",
  
  /* 01 */ "impossibile leggere il margine superiore\n",
  
  /* 02 */ "impossibile espellere il margine inferiore\n", /* FIXME: che vuol dire "clear bottom margin"? */
  
  /* 03 */ "impossibile leggere la riga successiva\n",
  
  /* 04 */ "impossibile allocare memoria per il PPA\n",
  
  /* 05 */ "errore invocando la funzione",   /*followed by function name*/
  
  /* 06 */ "codice di ritorno dello scambio sconosciuto\n", /* FIXME: "switch" = "scambio"?? */

  
  /* from ppa.c */

  /* 07 */ "troncati i dati del beccuccio a 6 righe, (erano ", /* followed by number */ /* FIXME: nozzle data?? */
  
  /* messages from image.c */
  
  /* 08 */ "impossibile creare il file gamma.ppm. Controllare i permessi.\n",
  
  /* 09 */ "Input troncato - File terminato durante la lettura dell'immagine:", /*followed by format */
  
  /* 10 */ "Errore critico  - impossibile leggere l'immagine in ingresso\n",
  
  /* 11 */ "Errore critico - l'immagine in ingresso ha pixel con valore > 255\n",
  
  /* 12 */ "Errore critico! Impossibile allocare memoria per il buffer!\n", /* FIXME: "memoria tampone"? */
  
  /* 13 */ "Errore critic! L'immagine in ingresso (pnm2ppa) non è un immagine PNM\n",
  
  /* 14 */                                             /* removed */
  
  /* 15 */ "La stampante non può stampare su carta in formato ", /*followed by size*/ 
  
  /* 16 */ "indice colore errato: il massimo è 255, il file contiene ", /*followed by value */
  
  /* messages from ppa.c and pnm2ppa.c */
  
  /* 17 */ "Versione della stampante sconosciuta\n", 
  
  /* 18 */ "Impossibile allocare memoria per i dati compressi\n",
  
  /* 19 */ "Comincia il lavoro di stampa\n",
  
  /* 20 */   "Stampa in corso, pagina ", /*followed by page number*/
  
  /* 21 */ "Composta la pagina ",  /*followed by page number*/
  
  /* 22 */ "Nessuna pagina stampata!\n",
  
  /* 23 */  "Stampa completata correttamente.\n",
  
  /* 24 */ "Informazioni dettagliate in console\n",
  
  /* 25 */ "Configurare la stampante in /etc/pnm2ppa.conf\n",
  
  /* 26 */ "impossibile aprire il file di configurazione\n",  

  /* 27 */ "formato della carta sconosciuto\n",

  /* 28 */ "parametro non riconosciuto", /*not followed by /n */

  /* 29 */ "alla linea", /* followed by line number */

  /* 30 */  "errore nel file di configurazione",  /* not followed  by \n */

  /* 31 */ "Errore critico: argomento sconosciuto\n" ,  

  /* 32 */ "Impossibile aprire il file di uscita PPA\n", 
  
  /* 33 */ "Impossibile aprire il file di ingresso PNM\n", 

  /* 34 */ "versione e data di compilazione:", /*followed by version number and date */

  /* 35 */ "Color correction curve (Gamma) file exists, but is corrupt.\n",

  /* 36 */ "POSSIBLE BUFFER OVERFLOW ATTACK!!: length of a path specified as an option exceeds", /*followed by maximum allowed size */
    "ERRORE CRITICO: messaggio di errore impossibile (bug nel programma)\n"





