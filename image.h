/***************************************************************************
    image.h  -  prototypes and stuff for image.c
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 1998-2000 by pnm2ppa project
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

#ifndef _IMAGE_H
#define _IMAGE_H

#include <stdio.h>

typedef struct
{
    FILE *fptr;
  enum
  { none, P1_PBM, P2_PGM, P3_PPM, P4_PBMRAW, P5_PGMRAW, P6_PPMRAW }
  version;
  int width, height;
  int blackCurLine, colorCurLine, bufferCurLine;
  unsigned char *LineType;
  int left, right, top, bottom;
  void *revdata;		/* used to stash a line read too early */
  int unread;
  
  /* buffer */
  int bufsize;		/* size of buffer in bytes */
  int bufpos;			/* start position in buffer */
  int buflines;		/* number of lines in the buffer */
  enum
  {
    empty = 0,
    bitmap,
    color,
    grey
  }
  buftype;			/* 0=empty, 1=black, 2=color, grey */
  unsigned char *palette;
  int ncols;
  unsigned char *buffer[4];
  int colors;                 /* for ppm and pgm formats (usually 255)  */
}
image_t;

/* values for image->LineType descriptions */
#define READLINE   0x01    /* line has been read */
#define NONBLANK   0x02    /* non blank line */
#define BLACKLINE  0x04    /* line has black ink */
#define COLORLINE  0x08    /* line has color ink */


int initImage (image_t *, FILE *);

/* reads a single line into char* */
int im_black_readline (image_t *, unsigned char *data[], int offset);

/* reads a single line of color */
int im_color_readline (image_t *, unsigned char *data[], int offset);

/* pushes a single line back */
void im_unreadline (image_t *, void *);

/* produces color calibration page  */
int ProduceGamma (image_t *, unsigned char *line );

#endif /* ; */




