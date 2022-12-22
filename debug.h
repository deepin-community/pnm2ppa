/***************************************************************************
                          debug.h  -  debug header
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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUG

/* debugging macros */

/* debugging routines */
void DPRINTF (char *format, ...);

#else

/* removal routines  */

#define DPRINTF		if(1){}else printf

#endif

#endif
