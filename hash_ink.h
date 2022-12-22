/*****************************************************************
     hash_ink.h - contains hashed version of palette.h data
                              -----------------
     begin          : Tue Jan 24 2000
     copyright      : (C) 2000 by the pmn2ppa project
     author         : Klamer Schutte
     email          : Schutte@fel.tno.nl
******************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HASH_INK_H
#define HASH_INK_H "$Id: hash_ink.h,v 1.1 2000/01/26 22:37:28 klamer Exp $"

/*
 * $Log: hash_ink.h,v $
 * Revision 1.1  2000/01/26 22:37:28  klamer
 * Added hashing support in FS_Color_Dither. Gives (for file processed)
 * 6 fold speedup for normal mode, and is 1.5 times quicker with --eco
 * mode. (CPU time needed -- printer is slower on my computer.)
 * Note that a trade-off between memory and CPU usage is made; large
 * arrays are generated in hash_ink.c. A pending optimization is to reduce the
 * entries in size; the integer size object can be packed to unsigned chars,
 * and the values of x y z even into a single char -- reducing the size of that
 * structure from 7 32 bit words (on i386) to 2 words...
 * Printout results should not change for the code inserted now;
 * however, marginal differences may exist as the distance (defined by
 * color metric) is the same, but multiple solutions might give the
 * same distance... Note that the error diffusion dithering should correct
 * this in nearby pixels.
 *
 */

struct hash_ink
{
  int             data[3];
  int             x, y, z;
  struct hash_ink *next;
};

extern struct hash_ink *arr_max4[], *arr_max1[];
extern int shift4, shift1;

#endif








