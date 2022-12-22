/***************************************************************************
    dither.h  -  function prototypes for dither.c
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

#ifndef _DITHER_H
#define _DITHER_H

void FS_Color_Dither (image_t * im, unsigned char *line);
void HT_Black_Dither (image_t * im, unsigned char *line);
void HT_Color_Dither (image_t * im, unsigned char *line);

#endif /* ; */
