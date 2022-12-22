/***************************************************************************
    ppa_syslog.h  -  prototypes and stuff for syslog.c
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
/*
  syslog.h
  modified Duncan Haldane <duncan_haldane@users.sourceforge.net>
  Oct 2000. all syslog messages are now wrapped.
*/

#ifndef _PNM2PPA_SYSLOG_H_
#define _PNM2PPA_SYSLOG_H_


#ifdef __NO_SYSLOG__
/* (included only for compatibility, if syslog.h is absent): */
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

#else
#include <syslog.h>
#endif

char syslog_message[128];
extern size_t message_size ;

void wrap_syslog( int, char *, char *);
void wrap_openlog( char *, int );
void wrap_closelog (void);

#ifdef __PNM2PPA_C__
size_t message_size = 128;
#endif


#endif













