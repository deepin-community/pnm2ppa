/***************************************************************************
    text-en.h  -  contains the english strings
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 2000 by Andrew van der Stock
    email                : ajv@greebo.net
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

/* 00 */ "could not malloc data storage\n",
  
  /* 01 */ "could not read top margin\n",
  
  /* 02 */ "could not clear bottom margin\n",
  
  /* 03 */ "could not read next line\n",
  
  /* 04 */ "could not malloc ppa storage\n",
  
  /* 05 */ "error calling",   /*followed by function name*/
  
  /* 06 */ "unknown return code in switch\n",
  
  /* from ppa.c */

  /* 07 */ "truncating nozzle data to 6 rows, (was ", /* followed by number*/
  
  /* messages from image.c */
  
  /* 08 */ "Could not create gamma.ppm file. Check permissions\n",
  
  /* 09 */ "Truncated input - EOF while reading image:", /*followed by format*/
  
  /* 10 */ "Fatal error  - failed to read input image\n",
  
  /* 11 */ "Fatal error - input image has pixel with value > 255\n",
  
  /* 12 */ "Fatal error! Unable to allocate buffer memory!\n",
  
  /* 13 */ "Fatal error! Input image (pnm2ppa) is not a PNM image\n",
  
  /* 14 */  /* (removed) */
  
  /* 15 */ "Printer cannot print on paper size ", /*followed by size*/ 
  
  /* 16 */ "maximum PNM pixel value must be 255, but is: ", /*followed by value*/
  
  /* messages from ppa.c and pnm2ppa.c*/
  
  /* 17 */ "Unknown Printer version\n", 
  
  /* 18 */ " Could not malloc storage for compressed data\n",
  
  /* 19 */ "Starting print job\n",
  
  /* 20 */   "Printing  page ", /*followed by page number*/
  
  /* 21 */ "Finished rendering page ",  /*followed by page number*/
  
  /* 22 */ "No pages printed!\n",
  
  /* 23 */  "Print job completed successfully.\n",
  
  /* 24 */ "Verbose logging to console\n",
  
  /* 25 */ "Please configure your printer in /etc/pnm2ppa.conf\n",
  
  /* 26 */ "couldn't open config file \n",

  /* 27 */ "unknown paper size\n",  

  /* 28 */ "unrecognized parameter",  /*not followed by \n */

  /* 29 */ "at line", /* followed by line number */ 

  /* 30 */  "error parsing config file",  /* not followed  by \n */

  /* 31 */ "Fatal: unknown argument\n" ,  

  /* 32 */ "Could not open PPA output file\n", 
  
  /* 33 */ "Could not open PNM input file\n",  

  /* 34 */ "version and build date:", /*followed by version number and date */

  /* 35 */ "Color correction curve (Gamma) file exists, but is corrupt.\n",

  /* 36 */ "POSSIBLE BUFFER OVERFLOW ATTACK!!: length of a path specified as an option exceeds", /*followed by maximum allowed size */


    "FATAL: Bad error message\n"






