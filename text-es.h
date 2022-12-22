/***************************************************************************
    text-es.h  -  contains the Spanish strings - Contiene las cadenas es español
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 2000 by Andrew van der Stock
    email                : ajv@greebo.net

    Translated by Javier Sedano (jsedano@it.uc3m.es), Wed Apr 12 11:10:14 CEST 2000,
    Spanish.
    Traducido por Javier Sedano (jsedano@it.uc3m.es), Wed Apr 12 11:10:14 CEST 2000,
    al español.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Este programa es software libre; puedes redistribuirlo y/o            *
 *   modificarlo bajo los terminos de la GNU General Public License        *
 *   (Licencia Publica General GNU) tal y como es publicada por la Free    *
 *   Software Fundation (Fundacion de Software Libre); la version 2 de la  *
 *   Licencia, o (a tu eleccion) cualquier version anterior.               *
 ***************************************************************************/

/* messages from cutswath.c */

/* 00 */ "no pude reservar memoria para almacen de datos\n",
  
  /* 01 */ "no pude leer el margen superior\n",
  
  /* 02 */ "no pude limpiar el margen inferior\n",
  
  /* 03 */ "no pude leer la siguiente linea\n",
  
  /* 04 */ "no pude reservar memoria para almacenar el ppa\n",
  
  /* 05 */ "error invocando",   /*followed by function name*/
  
  /* 06 */ "codigo devuelto por switch desconocido\n",
  
  /* from ppa.c */

  /* 07 */ "truncando datos de las boquillas a 6 filas, (era ", /* followed by number */
  
  /* messages from image.c */
  
  /* 08 */ "No pude crear el archivo gamma.ppm . Comprueba los permisos\n",
  
  /* 09 */ "Entrada truncada - EOF mientras leia la imagen:", /*followed by format */
  
  /* 10 */ "Error fatal - imposible leer imagen de entrada\n",
  
  /* 11 */ "Error fatal - la imagen de entrada tiene un pixel que vale mas de 255\n",
  
  /* 12 */ "Error fatal! Imposible reservar buffer de memoria!\n",
  
  /* 13 */ "Error fatal! La imagen de entrada (pnm2ppa) no es una imagen PNM\n",
  
  /* 14 */                                          /* removed */
  
  /* 15 */ "La impresora no puede imprimir en tamaño de papel ", /*followed by size */
  
  /* 16 */ "el maximo calor para un pixel PNM debe ser 255, pero es: ", /*followed by value */
  
  /* messages from ppa.c and pnm2ppa.c */
  
  /* 17 */ "Version de impresora desconocida\n",
  
  /* 18 */ "No pude reservar memoria para los datos comprimidos C\n",
  
  /* 19 */ "Comenzando trabajo de impresion\n",
  
  /* 20 */ "Imprimiendo pagina ", /*followed by page number */
  
  /* 21 */ "Terminado renderizado de la pagina ",  /*followed by page number*/
  
  /* 22 */ "No imprimi ninguna pagina!\n",
  
  /* 23 */ "Trabajo de impresion completado con exito.\n",
  
  /* 24 */ "Registrando salida en la consola\n",
  
  /* 25 */ "Por favor, configura tu impresora en /etc/pnm2ppa.conf\n",
  
  /* 26 */ "no pude abrir el archivo de configuracion\n",

  /* 27 */ "tamaño de papel desconocido\n", 

  /* 28 */ "parametro desconocido",  /*not followed by \n*/ 

  /* 29 */ "en la linea", /* followed by line number */ 

  /* 30 */ "error procesando el archivo de configuracionco",  /* not followed  by \n */

  /* 31 */ "Critico: argumento desconocido\n" ,  

  /* 32 */ "No pude abrir el archivo de salida PPA\n",
  
  /* 33 */ "No pude abrir el archivo de entrada PNM\n", 

  /* 34 */ "version y fecha de compilacion:", /*followed by version number and date */

  /* 35 */ "El archivo de correcion de color (gamma) existe, pero esta corrupto.\n",

  /* 36 */ "POSSIBLE BUFFER OVERFLOW ATTACK!!: length of a path specified as an option exceeds", /*followed by maximum allowed size */
    "CRITICO: Mensaje de error incorrecto\n"
