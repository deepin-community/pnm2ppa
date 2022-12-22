/***************************************************************************
    pnm2ppa.h  -  main program header file
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

#ifndef _PNM2PPA_H_
#define _PNM2PPA_H_

#ifdef __PNM2PPA_C__

static char *defaultcfgfile = SYSCONFDIR "/pnm2ppa.conf";
static char *defaultgammafile = SYSCONFDIR "/pnm2ppa.gamma";

static ppaPrinter_t printer;

static struct option long_opts[] = {
    {"bw", 0, 0, 0},
    {"fd", 0, 0, 0},
    {"version", 0, 0, 0},
    {"help", 0, 0, 'h'},
    {"verbose", 0, 0, 0},
    {"eco", 0, 0, 0},
    {"noGamma", 0, 0, 0},
    {"uni", 0, 0, 0 },
    {"bi", 0, 0, 0 },
    {"dpi300", 0, 0, 0},
    {0, 0, 0, 0}
};

#endif

int ppa_print_page (ppaPrinter_t *printer, image_t *image);
int print_pnm (FILE *fptr);
void set_printer_specific_defaults (void);
void show_usage (char *prog);
void parm_version (char *arg);
void parm_iversion (int arg);
void dump_config (void);
void read_config_file (char *fname);
char * readPath( char * );
int main (int argc, char *argv[]);

#endif


