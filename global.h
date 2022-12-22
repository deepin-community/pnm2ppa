/***************************************************************************
    global.h  -  contains all the global variables for the entire program
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

/*
 * Global Variables 
 */

/* now provided by the 'configure' script */
/* #define VERSION "1.12 (November 19, 2002)" */

/*
 * Global types
 */

typedef enum
{ false = 0, true = 1 }
BOOLEAN;

/* Paper sizes in 600ths of an inch. */

/* Default paper sizes: Letter, Legal, and A4. */

/* US Letter is 8.5 in by 11 in */

#define LETTERWIDTH  (5100)
#define LETTERHEIGHT (6600)

/* US Legal is 8.5 in by 14 in */

#define LEGALWIDTH  (5100)
#define LEGALHEIGHT (8400)

/* A4 is 210 mm by 297 mm == 8.27 in by 11.69 in */

#define A4WIDTH  (4960)
#define A4HEIGHT (7016)

#define MAXBUFFLINES 300

#ifdef __PNM2PPA_C__
int gEnh_curve_r[256], gEnh_curve_g[256], gEnh_curve_b[256];

int gMaxPass;
BOOLEAN gFastMode;

int gWidth;			/* width and height in 600ths of an inch */
int gHeight;          
int gminWidth;                  /* max and min values allowed by printer */
int gmaxWidth;
int gminHeight;
int gmaxHeight;

char gFormat[12];                /*  string describing the  image format */


BOOLEAN gPixmapMode;		/* Defines if printing text or photo
				 * (enable/disable black cartridge) */

BOOLEAN gColorMode;		/* enable/disable color cartridge */

BOOLEAN gUnimode;                /*  force unidirectional print sweeps */
BOOLEAN gTerminate;              /*  force clean termination of print run*/

int *gErrVec;
int *gErrVec_bw;

BOOLEAN gGammaMode;
BOOLEAN gEcoMode;

int gBlackness;   /* 1,2,3, or 4: black ink density adjustment */
BOOLEAN gBlack, gCyan, gMagenta, gYellow; /* switch off inks */

BOOLEAN gSilent, gVerbose ;   /* switch off syslog, messages to stderr */

int gByteCount;                    /* 1 for pbm/pgm input, 3 for ppm */

BOOLEAN gCalibrate;                /* true when printing calibration pages */
BOOLEAN g300;                      /* assume 300dpi input when true */
BOOLEAN gCompress;                  /* enable compressed (color) sweep data */
BOOLEAN gLogInfo;                   /* enable informational syslog messages */
#else
#ifndef __CALIBRATE_C__
extern int gMaxPass;
extern int gWidth;
extern int gHeight;
extern int gminWidth;
extern int gmaxWidth;
extern int gminHeight;
extern int gmaxHeight;
extern char gFormat[12];
extern int gBlackness ; 
extern BOOLEAN gUnimode ;
extern BOOLEAN gTerminate ;
extern BOOLEAN gColorMode;
extern BOOLEAN gEcoMode;
extern int gEnh_curve_r[256], gEnh_curve_g[256], gEnh_curve_b[256];
extern BOOLEAN gFastMode;
extern int *gErrVec;
extern int *gErrVec_bw;
extern BOOLEAN gGammaMode;
extern BOOLEAN gPixmapMode;
extern BOOLEAN gBlack, gCyan, gMagenta, gYellow;
extern BOOLEAN gSilent, gVerbose;
extern int gByteCount;
extern BOOLEAN gCalibrate;
extern BOOLEAN g300;
extern BOOLEAN gCompress;
extern BOOLEAN gLogInfo;
#endif
#endif

