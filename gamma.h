/***************************************************************************
    gamma.h  -  prototypes and stuff for gamma.c
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

#ifndef _GAMMA_H
#define _GAMMA_H

#include "image.h"

/* empirical color correction curve */
int Make_GammaCurve ( double , double , double );

/* array of trial gamma values */
double GammaValue ( int );

/* produces color calibration page  */
int ProduceGamma (image_t *, unsigned char *line );

#endif 




