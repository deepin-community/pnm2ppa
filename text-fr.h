/***************************************************************************
    text-fr.h  -  contains the french  strings
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 2000 by Andrew van der Stock
    email                : ajv@greebo.net
    translator           : b.alix@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* messages de cutswath.c */

  /* 00 */ "ne peut allouer de mémoire pour les données\n",
  
  /* 01 */ "ne peut lire la marge supérieure\n",
  
  /* 02 */ "ne peut effacer la marge inférieure\n",
  
  /* 03 */ "ne peut lire la ligne suivante\n",
  
  /* 04 */ "ne peut allouer de mémoire pour ppa\n",
  
    /* 05 */ "erreur lors de l'appel à",   /*suivi du nom de la fonction*/
  
  /* 06 */ "code retour inconnu lors du switch\n",
  
    /* from ppa.c */

    /* 07 */ "troncature des données buses à 6 lignes, (initiallement ", /*suivi du nombre */
  
    /* messages de image.c */
  
  /* 08 */ "ne peut créer le fichier gamma.ppm . Vérifiez les permissions\n",
  
    /* 09 */ "Entrée tronquée - EOF pendant la lecture de l'image:", /*suivi du format */
  
  /* 10 */ "Erreur fatale - impossible de lire l'image en entrée\n",
  
  /* 11 */ "Erreur fatale - l'image en entrée a un pixel d'une valeur > 255\n",
  
  /* 12 */ "Erreur fatale! Impossible d'allouer un buffer mémoire!\n",
  
  /* 13 */ "Erreur fatale! L'image en entrée (pnm2ppa) n'est pas un image PNM\n",
  
    /* 14 */                                          /* removed */
  
    /* 15 */ "L'imprimante ne peut imprimer sur cette taille de papier ", /*suivi de la taille */ 
  
    /* 16 */ "la valeur maximale d'un pixel PNM devrait être 255, mais est : ", /*suivi de la valeur */
  
    /* messages de ppa.c et pnm2ppa.c */
  
  /* 17 */ "Version d'imprimante inconnue\n", 
  
  /* 18 */ " Ne peut allouer de mémoire pour les données compressées\n",
  
  /* 19 */ "Début de la tâche d'impression\n",
  
    /* 20 */ "Impression de la page ", /*suivi du numéro de page*/
  
    /* 21 */ "Fin du rendu de la page ",  /*suivi du numéro de page*/
   
  /* 22 */ "Aucune page imprimée!\n",
  
  /* 23 */ "Tâche d'impression terminée avec succès.\n",
  
  /* 24 */ "messages affichés sur la console\n",
  
  /* 25 */ "Veuillez configurer votre imprimante dans /etc/pnm2ppa.conf\n",
  
  /* 26 */ "ne peut ouvrir le fichier\n", 

  /* 27 */ "taille de papier inconnue\n", 

    /* 28 */ "paramètre non reconnu", /*non suivi de \n*/

    /* 29 */ "à la ligne", /* suivi du numéro de ligne*/

    /* 30 */ "erreur de traitement du fichier de configuration",  /*non suivi de \n */

  /* 31 */ "Fatal: argument inconnu\n" ,  

  /* 32 */ "Ne peut ouvrir le fichier PPA en sortie\n",
  
  /* 33 */ "Ne peut ouvrir le fichier PNM en entrée\n", 

    /* 34 */ "version et date de création:", /*suivi du numéro de version et de sa date */

  /* 35 */ "Color correction curve (Gamma) file exists, but is corrupt.\n",

    /* 36 */ "POSSIBLE BUFFER OVERFLOW ATTACK!!: length of a path specified as an option exceeds", /*followed by maximum allowed size */

    "FATAL: mauvais message d'erreur\n"

