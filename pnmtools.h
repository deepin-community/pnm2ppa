/***************************************************************************
    pnmtools.h  -  headers for pnmtools.c . 
                             -------------------
    begin                : Sat Oct 14,  2000
    copyright            : (C) 2000 by pnm2ppa project
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

#ifndef _PNMTOOLS_H
#define _PNMTOOLS_H

void setpixel(int, int, unsigned char );

void setplus(int, int, int );

void setx(int, int, int );

void setblock(int, int, int );

void setchar(int, int, char, int );

void setstring(int , int , char* , int);

void setstringv(int ,int ,char* ,int );

void setCG(int x,int y);

void setsquare(int , int , int , int , int , int );


extern int Width;      /* width and height in 600ths of an inch */
extern int Height;
#if 0
extern int Pwidth;     /* width in bytes */
#endif
extern char *color_bitmap;
extern int BitMap_topline;    /* top line of bitmap */
extern int BitMap_bottomline; /* bottom line of bitmap */
extern int BitMap_Height;     /* height for malloc */
extern int BitMap_Width;      /* width for malloc */

#ifdef __CALIBRATE_PPA_C__
int Width;      /* width and height in 600ths of an inch */
int Height;
#if 0
int Pwidth;     /* width in bytes */
int Pheight;    /* height for malloc */
#endif
char *color_bitmap;
int BitMap_topline;    /* top line of bitmap */
int BitMap_bottomline; /* bottom line of bitmap */
int BitMap_Height;     /* height for malloc */
int BitMap_Width;      /* width for malloc */

#endif

#endif











