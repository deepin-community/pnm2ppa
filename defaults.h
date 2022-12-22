/***************************************************************************
    defaults.h  -  contains model-specific defaults for various parameters
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 2000 by the pnm2ppa project
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DEFAULTS_H
#define _DEFAULTS_H

/* Refer to CALIBRATION file about these settings */

#define MAXPRNNAMELEN	64

typedef enum
{
    NOPRINTER = 0xff,
    HP7X0 = 0,
    HP820,
    HP1000
}
printerModel_t;

typedef struct
{
  printerModel_t model;
  char name[MAXPRNNAMELEN];
  char friendlyname[MAXPRNNAMELEN];
  int ColBwOffsX;
  int ColBwOffsY;
  int x_offset;
  int y_offset;
  int r2l_bw_offset;
  int r2l_col_offset;
  int top_margin;
  int bottom_margin;
  int left_margin;
  int right_margin;
  int marg_diff;
  size_t bufsize;
  int min_pagewidth;
  int max_pagewidth;
  int min_pageheight;
  int max_pageheight;
}
printerDefault_t;

#ifdef __PNM2PPA_C__

/* we only initialise the once */

printerDefault_t gPrinterDefaults[] = {
    {
     HP7X0,
     "HP7x0",
     "HP 710C, 712C, 720C, 722C Series Printers",
     -5,			/* colxoff */
     43,			/* colyoff */
     160,			/* x_offset */
     50,			/* y_offset */
     0,                         /* r2l_bw_offset */
     0,                        /* r2l_col_offset */
     10,			/* top_margin */
     150,			/* bottom_margin */
     10,			/* left_margin */
     10,			/* right_margin */
     2,				/* margin difference */
     200 * 1024,		/* buffer size */
     1800,                      /* min pagewidth   3"  */
     5100,                      /* max pagewidth   8.5"*/
     1800,                      /* min pageheight  3"  */
     8400                       /* max pageheight 14"  */
     },
    {
     HP820,
     "HP820",
     "HP 820 Series Printers",
     35,			/* colxoff */
     43,			/* colyoff */
     160,			/* x_offset */
     50,			/* y_offset */
     0,                         /* r2l_bw_offset */
     0,                        /* r2l_col_offset */
     80,			/* top_margin */
     150,			/* bottom_margin */
     80,			/* left_margin */
     80,			/* right_margin */
     0x62,			/* margin difference */
     200 * 1024,	       	/* buffer size */
     1800,                      /* min pagewidth   3"  */
     5100,                      /* max pagewidth   8.5"*/
     1800,                      /* min pageheight  3"  */
     8400                       /* max pageheight 14"  */
     },
    {
     HP1000,
     "HP1000",
     "HP 1000 Series Printers",
     35,			/* colxoff */
     43,			/* colyoff */
    160,			/* x_offset */
     50,			/* y_offset */
      0,                        /* r2l_bw_offset */
      0,                        /* r2l_col_offset */
     10,			/* top_margin */
     150,			/* bottom_margin */
     10,			/* left_margin */
     10,			/* right_margin */
     0x62,			/* margin difference */
     200 * 1024,	       	/* buffer size */
     2400,                      /* min pagewidth   4" */
     7800,                      /* max pagewidth  13" */
     3600,                      /* min pageheight  6" */
     11400                      /* max pageheight 19" */
     },
    {
     NOPRINTER,
     "",
     "",
     0,				/* colxoff */
     0,				/* colyoff */
     0,				/* x_off */
     0,				/* y_off */
     0,                         /* r2l_bw_offset */
     0,                         /* r2l_col_offset */
     0,				/* top */
     0,				/* bottom */
     0,				/* left */
     0,				/* right */
     0,				/* margin difference */
     0,				/* buffer size */
     0,                         /* min pagewidth  */
     0,                         /* max pagewidth  */
     0,                         /* min pageheight */
     0                          /* max pageheight */
     } 
};

#else
extern printerDefault_t *gPrinterDefaults;
#endif

#endif







