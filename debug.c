/***************************************************************************
    debug.c  -  debug support routines for pnm2ppa
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef DEBUG

#include <stdio.h>
#include <stdarg.h>

#define __DEBUG_C__

#include "debug.h"

void
DPRINTF (char *format, ...)
{
    va_list p;

    va_start (p, format);
    vfprintf (stderr, format, p);
    va_end (p);
}

#endif
