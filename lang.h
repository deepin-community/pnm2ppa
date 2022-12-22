/***************************************************************************
    lang.h  -  contains the language selectors for I18N purposes
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 1999-2000 by Andrew van der Stock
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

#ifndef __LANG_H__
#define __LANG_H__

typedef enum
{
    E_CS_BADMALLOC,             /*  00 */
    E_CS_BADTOPMARG,            /*  01 */
    E_CS_BADBOTMARG,            /*  02 */
    E_CS_BADNEXTLINE,           /*  03 */
    E_CS_BADPPAMALLOC,          /*  04 */
    E_CS_ERROR,                 /*  05 */
    E_CS_BADSWITCH,             /*  06 */
    W_PPA_NOZZLE,               /*  07 */
    E_IM_BADGAMMA,              /*  08 */
    E_IM_EOFREADLINE,           /*  09 */
    E_IM_BADREADLINE,           /*  10 */
    E_IM_BADIMAGE,              /*  11 */
    E_BADMALLOC,                /*  12 */
    E_IM_BADFORMAT,             /*  13 */
                                /*  14  (removed) */
    E_IM_BADPAPERSIZE,          /*  15 */
    E_IM_BADMAXVALUE,           /*  16 */
    E_PPA_UNKNOWN,              /*  17 */    
    E_PPA_BADMALLOC,            /*  18 */
    LOG_START,                  /*  19 */
    LOG_PAGE,                   /*  20 */
    LOG_FINISH,                 /*  21 */
    LOG_NOPAGES,                /*  22 */
    LOG_SUCCESS,                /*  23 */
    LOG_VERBOSE,                /*  24 */
    E_CONFIG,                   /*  25 */
    E_BADCONFIGFILE,            /*  26 */
    E_BAD_PAPER,                /*  27 */
    E_BADPARM,                  /*  28 */
    LINE,                       /*  29 */
    E_PARSE_CONFIG,             /*  30 */
    E_UNKNOWN_ARG,              /*  31 */
    E_BAD_OUTPUT,               /*  32 */
    E_BAD_INPUT,                /*  33 */
    LOG_VERSION,                /*  34 */
    E_BAD_GAMMAFILE,            /*  35 */
    E_BAD_PATH     ,            /*  36 */

    E_LANGNOMESS		/* placeholder */
}
mess_t;


#ifdef __PNM2PPA_C__
/* only defined the once */
char *gMessages[] = {

#ifdef LANG_CZ
#include "text-cz.h"
#endif

#ifdef LANG_EN
#include "text-en.h"
#endif

#ifdef LANG_ES
#include "text-es.h"
#endif

#ifdef LANG_FR
#include "text-fr.h"
#endif

#ifdef LANG_IT
#include "text-it.h"
#endif

#ifdef LANG_NL
#include "text-nl.h"
#endif
};

#else
/* make the messages available to other files */
extern char *gMessages[];
#endif

#endif
