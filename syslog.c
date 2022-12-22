/***************************************************************************
    syslog.c  -  handles syslog messages
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/*Michael Mancini <gwaihir@email.com>
  16 Jan 2000
  BeOS syslog() wrapper
  syslog.c
  modified Duncan Haldane <duncan_haldane@users.sourceforge.net>
  Oct 2000. all syslog messages arrive here wrapped.  
*/

#include  <stdio.h>
#include "ppa_syslog.h"
#include "global.h"

BOOLEAN verbose = false ;

/*
  For some reason, BeOS doesn't seem to implement syslog(), even though it is
  included with the system.  Wierd.  This wraps the syslog functions used
  and writes the info to the stderr stream, if __NO_SYSLOG__ is defined.   
*/


void 
wrap_syslog(int log_pri, char *fmt, char *message )
{
  if  (!(gSilent)) {
#ifdef __NO_SYSLOG__  
      gVerbose = true;
#else
      if (gLogInfo || log_pri != LOG_INFO)    
	syslog ( log_pri , "%s", message );
      else if(verbose)
	fprintf(stderr,"pnm2ppa: %s",message);
#endif
  }

  if (gVerbose)
    {
      /* Send the syslog data to the stderr stream */
      fprintf(stderr,"pnm2ppa: %s",message);
    }
  return ;
}

void wrap_openlog( char *ident, int level )
{    
#ifndef __NO_SYSLOG__  
  if ( level ) {
      /* level 1  also sends messages  to stderr */
#ifdef __NO_LOG_PERROR__
    /* for systems (e.g. Solaris) where LOG_PERROR is not valid */
      openlog ( ident, LOG_CONS | LOG_PID, LOG_LPR);
      gVerbose = true;
#else
      openlog ( ident, LOG_PERROR | LOG_CONS | LOG_PID, LOG_LPR);
      verbose = gVerbose;
      gVerbose = false;
#endif
  }
  else {
      /*  standard level 0, messages sent to syslog only */
      openlog ( ident, LOG_CONS | LOG_PID, LOG_LPR);
      verbose = gVerbose;
      gVerbose = false;
    }
#endif
  return;
}

void 
wrap_closelog( void)
{
#ifndef __NO_SYSLOG__  
  closelog();
#endif
  return;
}












