
/***************************************************************************
              ppa.h  -  PPA protocol definitions and routines
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 1998-2000 by the pnm2ppa project
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

#ifndef _PPA_H
#define _PPA_H

#include "defaults.h"

typedef struct
{
  FILE *fptr;
  printerModel_t version;	/* from defaults.h */
  int ColBwOffsX;
  int ColBwOffsY;
  int x_offset;
  int y_offset;
  int r2l_bw_offset;
  int r2l_col_offset;
  int marg_diff;
  int top_margin;
  int left_margin;
  int right_margin;
  int bottom_margin;
  int bufsize;
  int min_pagewidth;
  int max_pagewidth;
  int min_pageheight;
  int max_pageheight;
}
ppaPrinter_t;

typedef struct
{
  unsigned short DPI;
  unsigned short pins_used_d2;
  unsigned short unused_pins_p1;
  unsigned short first_pin;
  unsigned short left_margin;
  unsigned short right_margin;
  unsigned char nozzle_delay;
}
ppaNozzleData_t;

typedef struct ppaSweepData_s
{
  unsigned char *image_data;
  unsigned data_size;
  BOOLEAN in_color;
  enum
  { right_to_left, left_to_right, unknown}
  direction;
  int vertical_pos;
  unsigned short left_margin;
  unsigned short right_margin;
  unsigned char nozzle_data_size;
  ppaNozzleData_t *nozzle_data;
  struct ppaSweepData_s *next;	/* NULL indicates last print sweep */
  int first_data_line;
  int last_data_line;
  int pre_blanklines;
  int post_blanklines;
}
ppaSweepData_t;

typedef struct ppaSwathLimits_s
{
  int left;
  int right;
  int numlines;
  int non_blanklines;
  int pre_blanklines;
  int post_blanklines;
  int first_line;
  int last_line;
}
ppaSwathLimits_t;

typedef struct ppaPageLimits_s
{
  int left;
  int right;
  int top;
  int bottom;
}
ppaPageLimits_t;

void ppa_init_job (ppaPrinter_t *);
void ppa_init_page (ppaPrinter_t *);
void ppa_load_page (ppaPrinter_t *);
void ppa_eject_page (ppaPrinter_t *);
void ppa_end_print (ppaPrinter_t *);
void ppa_print_sweep (ppaPrinter_t *, ppaSweepData_t *);	/* prints one sweep */
void ppa_print_sweeps (ppaPrinter_t *, ppaSweepData_t *);	/* prints a linked-list of sweeps */

#endif








