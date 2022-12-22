/***********************************************************************
    cutswath.c  -  function to cut a swath of a PNM file for PPA printers
                         -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 1998-2002 by the pnm2ppa project
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define __CUTSWATH_C__

#include "ppa_syslog.h"
#include "global.h"
#include "debug.h"
#include "defaults.h"
#include "ppa.h"
#include "image.h"

#include "lang.h"

/* stores information of previous sweeps */
int prev_black_vpos , prev_black_dir;
int prev_color_vpos , prev_color_dir;

/* pointers to next sweep that will print, and waiting sweeps */
ppaSweepData_t *current_swath, *waiting_color, *waiting_black; 

/* stores info on  printable limits of page */
ppaPageLimits_t ColorPageLimits;
ppaPageLimits_t BlackPageLimits;

int 
cut_im_black_swath (image_t *image, ppaPrinter_t *printer,
		    ppaSweepData_t *sweep_data, int last_line);
int 
cut_im_color_swath (image_t *image, ppaPrinter_t *printer, 
		    ppaSweepData_t *sweep_data[4]);

/*
  Here, we should call cut_im_black_swath and cut_im_color_swath to begin.
  
  Then, we need to determine which of these swaths comes earlier on the page
  and print it first.  Then we should re-call the appropriate cut_im_*_swath
  function to refill that buffer and repeat this procedure.
  
  NEW (Feb 2002) : Finally, the sweep data is sent one sweep at a time to
  ppa_print_sweep(), where the PPA instructions are produced...
  
  This will make any further improvements much easier (i.e,
  staggering the print swaths, in high quality color mode).
*/

int 
ppa_print_page (ppaPrinter_t *printer, image_t *image) 
{
  ppaSweepData_t sweeps[6]; 
  ppaSweepData_t *prev = NULL, *curblack = NULL, *curcolor = NULL, 
    *color[4] = {NULL, NULL, NULL, NULL}, 
    *tmp = NULL, *unused = NULL, *waiting = NULL;
  int reload[2] = { 1, 1 };
  int done[2] = { 0, 0 };
  int found[2] = { 0, 0};
  int j, k;
  int last_dir = unknown;
  int last_line;
  int color_sweep = 0, color_todo = 0;
  int color_byte_width, black_byte_width;
  BOOLEAN NoPrint = gTerminate;  
  BOOLEAN color_done = false;
  ppaPageLimits_t * black_page_limit = &BlackPageLimits;
  ppaPageLimits_t * color_page_limit = &ColorPageLimits;
  /* read the data from the input file */
  

  /* initialize page limits for this document */  
  /* 
     vertical limits are in lines (1 line = 1dot at 600dpi),
     horizontal limits are in data bytes:
     1 color data byte = 8 dots at 300dpi,
     1 black data byte = 8 dots at 600dpi
  */

  image->left = 0;
  image->right = 0;
  image->top = 0;
  image->bottom = 0;
  if(printer->left_margin > 0) 
    image->left = printer->left_margin;
  if(printer->right_margin > 0)
    image->right = printer->right_margin;
  if(image->left >=  image->width  - image->right)
    NoPrint = true;
  if(printer->top_margin > 0)
    image->top = printer->top_margin;
  if(printer->bottom_margin > 0)
    image->bottom = printer->bottom_margin;
  if(image->top >=  image->height  - image->bottom)
    NoPrint = true;
  if(NoPrint)
    gTerminate = true;



  /* protect against invalid margin settings configured by users  */
  color_byte_width = (image->width / 2 + 7) / 8;
  color_page_limit->left =  printer->left_margin / 2 / 8;
  if ( color_page_limit->left < 0 ) 
    color_page_limit->left = 0 ;
  color_page_limit->right = (gWidth - printer->right_margin) / 2 / 8;
  if ( color_page_limit->right > color_byte_width  )
    color_page_limit->right =  color_byte_width ;
  color_page_limit->bottom = gHeight - printer->bottom_margin  ;
  if ( color_page_limit->bottom > gHeight)
    color_page_limit->bottom = gHeight ;
  if ( color_page_limit->bottom < 0 )
    color_page_limit->bottom = 0;
  color_page_limit->top = printer->top_margin ;
  if ( color_page_limit->top < 0 )
    color_page_limit->top = 0 ;
  if ( color_page_limit->top > color_page_limit->bottom )
    color_page_limit->top = color_page_limit->bottom ;

  black_byte_width = (image->width + 7) / 8;  
  black_page_limit->left = printer->left_margin / 8;
  if ( black_page_limit->left < 0 )
    black_page_limit->left = 0;
  black_page_limit->right = ( gWidth  - printer->right_margin ) / 8;
  if (black_page_limit->right > black_byte_width )
    black_page_limit->right = black_byte_width;
  black_page_limit->bottom = color_page_limit->bottom;
  black_page_limit->top = color_page_limit->top;

  /* create an array for storing info about each line on this page */
  
  if ((image->LineType = malloc (image->height)) == NULL) {
    snprintf(syslog_message,message_size,"ppa_print_page(): %s", 
	     gMessages[E_CS_BADMALLOC]);
    wrap_syslog (LOG_CRIT,"%s",syslog_message); 
    return 0;
  }
  memset(image->LineType, 0x00, image->height);

  /* note: a full-size (all nozzles) color swath is 64 even lines (300dpi); 
     a full-size (all nozzles) black swath is 300 lines (150 even + 150 odd);

     is is necessary to restrict black swath to 128 lines (64 even + 64 odd)
     on some printer models, when printing in dual inks, color+black ? 
     HP7x0:   NO, but there is some artifact at the begining of
     each black swath that does not immediately follow a black 
     swath.  (see the calibrate_ppa --align  output....)
     so leave the setting at 128 in gColorMode for now. 
     (duncan, 2000-10-24)
     HP820:  YES flashes lights ... (maybe  it will work if what
     produces the artifacts on HP7x0 is fixed..?)
     HP1000: ?
  */
  
  
  DPRINTF("ppa_print_page(): begin page\n");
  
  /* initialize global prev vertical position and direction  markers, etc. */
  prev_color_vpos = color_page_limit->top - 1;
  prev_black_vpos = black_page_limit->top - 1;
  prev_black_dir = prev_color_dir = unknown;
  
  for (k = 0; k < 6; k++) {
    sweeps[k].direction = unknown;
    sweeps[k].next = NULL;
    sweeps[k].image_data = NULL;
    sweeps[k].nozzle_data = NULL;
  }
  
  /* initialize sweep_data pointers */  
  curblack = &sweeps[0];
  unused = &sweeps[1];
  for (k = 0; k < gMaxPass; k++)
    color[k] = &sweeps[k+2];
  
  /* when the cut_im_*_swath functions are called with
      the sweep direction = "unknown", they will assign
      the direction.   
      if gMaxPass is even (i.e. != 1) , the initial
      value of color[0]->direction is ignored.
      the cut_im_*swath functions may also override the
      inital sweep directions in some other cases
      (e.g., unidirectional printing was requested).
      
      If gMaxPass is even, half of
      the gMaxPass color sweeps will be left_to_right,
      half will be  right_to_left.  In the present code
      (gMaxPass = 4) all these sweeps have the same
      vertical position.   ppa_print_page() currently
      is based on the assumption that if gMaxPass is even.
      each r2L color sweep has a  corresponding l2R sweep with
      the same vertical_pos.  If this changes, it should be
      reworked.
   */

  if ( !gColorMode){
    done[1] = 1;
    prev_color_vpos = color_page_limit->bottom +1;
  }
  DPRINTF ("ppa_print_page: chkpt 1\n");
  
  while (!done[0] || !done[1]){  


    /* to be safe, don't ever read beyond end of page */
    if(image->blackCurLine >= gHeight)
      done[0] = 1;
    if(image->colorCurLine >= gHeight)
      color_done = 1;
    if(color_done && !color_todo)
      done[1] = 1;

    /* create a color swath */
    if (reload[1] && !done[1]){
      reload[1] = 0;
      current_swath = prev;
      waiting_color = NULL;
      curcolor = NULL;
      if(curblack->image_data)
	waiting_black = curblack;
      else
	waiting_black = NULL;
      if (color_todo){
	color_sweep++;
	color_todo--;
	/* attempt to alternate direction */
	if (!gUnimode && prev ){
	    BOOLEAN switch_dir=false;
	    if ( !waiting ||
		 color[color_sweep]->vertical_pos <= waiting->vertical_pos){
	      if (color[color_sweep]->direction == prev->direction)
		switch_dir = true;
	    } else if (color[color_sweep]->direction == waiting->direction)
	      switch_dir = true;
	    
	    if (switch_dir){ 
	      for (j=1; j <= color_todo; j++){
		if (color[color_sweep+j]->vertical_pos >
		    color[color_sweep]->vertical_pos)
		  break;
		if (color[color_sweep+j]->direction != 
		    color[color_sweep]->direction)
		  {
		    tmp = color[color_sweep];
		    color[color_sweep] = color[color_sweep+j];
		    color[color_sweep+j] = tmp;
		    break;
		  }
	      }
	    }
	}
	curcolor = color[color_sweep];
	DPRINTF("*** loaded color[%d], dir=%d, vpos=%d, todo=%d\n",
		color_sweep,curcolor->direction, 
		curcolor->vertical_pos, color_todo);
#ifdef DEBUG
	for (k=0; k <= color_todo; k++){
	  DPRINTF("     %p [%d]   dir=%d vpos=%d color=%d\n",
		  color[color_sweep+k],k,
		  color[color_sweep+k]->direction,
		  color[color_sweep+k]->vertical_pos,
		  color[color_sweep+k]->in_color);
	}
#endif
      } else if (!color_done) 
	switch (cut_im_color_swath (image, printer, color)) {
	case 0:
	  snprintf(syslog_message,message_size, "ppa_print_page(): %s cut_im_color_swath()", 
		   gMessages[E_CS_ERROR]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	  break;
	  return 1;
	case 1:
	  DPRINTF("ppa_print_page: cut_im_color_swath returned 1\n");
	  color_done = true;
	  prev_color_vpos = color_page_limit->bottom +1;
	  prev_color_dir = unknown;;
	  break;
	case 2:
	  /* count and sort non-NULL sweeps */
	  color_todo = 0;
	  for (k=0; k < gMaxPass; k++){
	    if (color[k]->image_data)
	      color_todo++;
	    else {
	      for (j=k+1; j < gMaxPass; j++){
		if (color[j]->image_data){
		  tmp = color[j];
		  color[j] = color[k];
		  color[k] = tmp;
		  break;
		}
	      }
	    }
	    if (!color[k]->image_data)
	      break;
	  }
	  /* sort by vertical_pos */
	  for (k=0; k < color_todo; k++) {
	    int next = k;
	    for (j = k+1; j < color_todo; j++){
		if (color[j]->vertical_pos < color[next]->vertical_pos)
		  next = j;
	    }
	    tmp = color[k];
	    color[k] = color[next];
	    color[next] = tmp;
	  }
		/* attempt to alternate direction */
	  color_sweep = 0;
	  color_todo--;
	  if (!gUnimode && prev ){
	      BOOLEAN switch_dir=false;
	      if ( !waiting ||
		   color[color_sweep]->vertical_pos < waiting->vertical_pos){
		if (color[color_sweep]->direction == prev->direction)
		  switch_dir = true;
	      }
	      else if (color[color_sweep]->direction == waiting->direction)
		switch_dir = true;
	      
	      if (switch_dir){ 
		for (j=1; j <= color_todo; j++){
		  if (color[color_sweep+j]->vertical_pos >
		      color[color_sweep]->vertical_pos)
		    break;
		  if (color[color_sweep+j]->direction != 
		      color[color_sweep]->direction){
		    tmp = color[color_sweep];
		    color[color_sweep] = color[color_sweep+j];
		    color[color_sweep+j] = tmp;
		    break;
		  }
		}
	      }
	  }
	  curcolor = color[color_sweep];
	  found[1]=1;
	  DPRINTF("*** loaded color[%d], dir=%d, vpos=%d, todo=%d\n",
		  color_sweep,curcolor->direction, 
		  curcolor->vertical_pos, color_todo);
#ifdef DEBUG
	  for (k=0; k <= color_todo; k++){
	    DPRINTF("     %p [%d]   dir=%d vpos=%d color=%d\n",
		    color[color_sweep+k],k,
		    color[color_sweep+k]->direction,
		    color[color_sweep+k]->vertical_pos,
		    color[color_sweep+k]->in_color);
	  }
#endif
		break;
	case 3:
	  prev_color_dir = unknown;
	  prev_color_vpos = color_page_limit->top - 1;
	  reload[1]=1;
	  found[1]=0;
	  break;
	default:
	  snprintf(syslog_message,message_size,"ppa_print_page(): %s ",  
		   gMessages[E_CS_BADSWITCH]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	  return 1;
	}
    }
    

    /* create a black swath */
    if (reload[0]){
      reload[0] = 0;
      current_swath = prev;
      waiting_black = NULL;
      waiting_color = curcolor;
      if (gColorMode && !color_done)
	last_line = image->colorCurLine ;
      else
	last_line = image->height;
      switch (cut_im_black_swath (image, printer, curblack, last_line)){
      case 0:
	snprintf(syslog_message,message_size,"ppa_print_page(): %s cut_im_black_swath()", 
		 gMessages[E_CS_ERROR]);
	wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	return 1;
	break;
      case 1:
	done[0] = 1;
	prev_black_vpos = black_page_limit->bottom +1;
	prev_black_dir = unknown;;
	break;
      case 2:
	found[0]=1;
	DPRINTF("*** loaded black, dir=%d, vpos=%d\n",
		curblack->direction, curblack->vertical_pos);
#ifdef DEBUG
	DPRINTF("     %p [B]   dir=%d vpos=%d color=%d\n",
		curblack,
		curblack->direction,
		curblack->vertical_pos,
		curblack->in_color);
#endif
	break;
      case 3: 
	prev_black_vpos = black_page_limit->top -1;
	prev_black_dir = unknown;
	reload[0]=1;
	found[0]=0;
	break;
      default:
	snprintf(syslog_message,message_size,"ppa_print_page(): %s", 
		 gMessages[E_CS_BADSWITCH]);
	wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	return 1;
      }
    }
 
    if (done[0] && done[1])
      break;
    
    /* choose which swath (black or color) should print next */
    if ( (!done[0] && found[0] && 
	  (color_done || !found[1] ||
	   curblack->vertical_pos < curcolor->vertical_pos)) ) {
      /* print the current black swath, if it exists and has a 
	 vertical_pos that is earlier any current color swath,
	 that exists */

      DPRINTF("ppa_print_page: chkpt 2\n");
      
      reload[0] = 1;
      DPRINTF("ppa_print_page: chkpt 3\n");
      if (prev){
	prev->next = curblack;
	if (found[1] && !done[1])
	  waiting = curcolor;
	else
	  waiting = NULL;
#ifdef DEBUG
	DPRINTF("*** ppa_print_sweep 1, %p, dir=%d (last=%d),"
		"color=%d, vpos=%d, lines [%d-%d]\n",
		prev,prev->direction, last_dir,
		prev->in_color,prev->vertical_pos,
		prev->first_data_line,prev->last_data_line);
#endif
	if(!NoPrint)
	  ppa_print_sweep (printer, prev);
	if (!gUnimode && last_dir == prev->direction){
	  DPRINTF("***SWEEP DIRECTION DID NOT CHANGE!!!***\n");
	}
	last_dir = prev->direction;
	free (prev->image_data);
	free (prev->nozzle_data);
	prev->image_data = NULL;
	prev->nozzle_data = NULL;
	/* move the current sweep into the 'prev' position */
	tmp = curblack;
	curblack = prev;
	prev = tmp;
      } else {
	/* move the current sweep into the 'prev' position */
	prev = curblack;
	assert (unused);
	curblack = unused;
	unused = NULL;
      }
    } else if ( (!done[1] && found[1]) ) {
      /* print the current color swath, if it exists */
      DPRINTF("ppa_print_page: chkpt 4 \n");
      
      reload[1] = 1;
      
      if (prev) {
	prev->next = curcolor;
	if (found[0] && !done[0])
	  waiting = curblack;
	else
	  waiting = NULL;
#ifdef DEBUG
	DPRINTF("*** ppa_print_sweep 2, %p, dir=%d (last=%d),"
		  "color=%d, vpos=%d, lines [%d-%d]\n",
		prev,prev->direction, last_dir,
		  prev->in_color,prev->vertical_pos,
		  prev->first_data_line,prev->last_data_line);
#endif
	if(!NoPrint)
	  ppa_print_sweep (printer, prev);
	if (!gUnimode && last_dir == prev->direction){
	  DPRINTF("***SWEEP DIRECTION DID NOT CHANGE!!!***\n");
	}
	last_dir = prev->direction;
	free (prev->image_data);
	free (prev->nozzle_data);
	prev->image_data = NULL;
	prev->nozzle_data = NULL;
	color[color_sweep] = prev;
      } else {
	assert (unused);
	color[color_sweep] = unused;
	unused = NULL;
      }
      /* move the current sweep into the 'prev' position */	  
      prev = curcolor;
    }
    if(color_done && !color_todo)
      done[1] = 1;
  }
  
  DPRINTF ("ppa_print_page: chkpt 5\n");
  
  if (prev) {
    DPRINTF ("ppa_print_page: chkpt 6 about to clear prev\n");
    
    prev->next = NULL;
#ifdef DEBUG
    DPRINTF("*** ppa_print_sweep 3, %p, dir=%d (last=%d),"
	    "color=%d, vpos=%d, lines [%d-%d]\n",
	    prev,prev->direction, last_dir,
	    prev->in_color,prev->vertical_pos,
	    prev->first_data_line,prev->last_data_line);
#endif
    if(!NoPrint)
      ppa_print_sweep (printer, prev);
    if (!gUnimode && last_dir == prev->direction){
      DPRINTF("***SWEEP DIRECTION DID NOT CHANGE!!!***\n");
    }
    last_dir = prev->direction;
    free (prev->image_data);
    free (prev->nozzle_data);
    prev->image_data = NULL;
    prev->nozzle_data = NULL;
    DPRINTF ("ppa_print_page: chkpt 7 cleared prev\n");
  }
  
  DPRINTF ("ppa_print_page: chkpt 8 Finish.\n");
  
  for (k=0; k < gMaxPass; k++)
    free(image->buffer[k]);

  free(image->LineType);
  image->LineType = NULL;

  return 0;
}

/* cutswath functions */

unsigned char *
l2r_black_sweep(unsigned char *place, unsigned char *data,
		int data_width, ppaSwathLimits_t * swath_limits);
unsigned char *
r2l_black_sweep(unsigned char *place, unsigned char *data,
		int data_width, ppaSwathLimits_t * swath_limits);
unsigned char *
l2r_color_sweep(unsigned char *place, unsigned char *data,
		int data_width, ppaSwathLimits_t * swath_limits);
unsigned char *
r2l_color_sweep(unsigned char *place, unsigned char *data,
		int data_width, ppaSwathLimits_t * swath_limits);

int
black_nozzle_data(ppaSweepData_t * sweep_data, int numlines, int margin_diff);

int
color_nozzle_data(ppaSweepData_t * sweep_data, int numlines);

int
read_black_image( unsigned char *data[], image_t *image,
		  ppaPrinter_t *printer, int byte_width,
		  int vpos_shift, int maxlines,
		  ppaSwathLimits_t *swath_limits,
		  int last_line);

int
read_color_image( unsigned char *data[], image_t *image,
		  ppaPrinter_t *printer, int byte_width,
		  int vpos_shift, int maxlines,
		  ppaSwathLimits_t *swath_limits);


/* Upon successful completion, sweep_data->image_data and
   sweep_data->nozzle_data have been set to pointers which this routine
   malloc()'d. */

/* Upon successful completion, all members of *sweep_data have been set
   except  next. */

/* Returns: 0 if unsuccessful
            1 if successful, but with non-printing result (end of page)
            2 if successful, with printing result 
	    3 if unsuccessful, exceeded buffer size */

int
cut_im_black_swath (image_t *image, ppaPrinter_t *printer,
		    ppaSweepData_t *sweep_data, int last_line)
{
  unsigned char *data[4] = { NULL, NULL, NULL, NULL};
  unsigned char *ppa, *place, *maxplace;
  int width, byte_width = (image->width + 7) / 8;  
  int vpos_shift=600;   /* "vertical position" = line number - vpos_shift */
  int horizpos;
  ppaSwathLimits_t Swath_Limits;
  ppaSwathLimits_t *swath_limits = &Swath_Limits; 
  int maxlines = 300;   /* fixed by design of print head */

  /* return if we are not yet allowed to read next part of
     black image */
  if(image->blackCurLine >= last_line)
    return 3;

  /* 
     if we need to use "legacy" mode for printing Calibration pages
     produced by calibrate_ppa, set maxlines = 128 as in pnm2ppa-1.04.
  */
  if(gCalibrate && gColorMode)
    maxlines = 128;


  
  /* safeguard against the user freeing these */
  sweep_data->image_data = NULL;
  sweep_data->nozzle_data = NULL;
  
  
  if ((data[0] = malloc (byte_width * maxlines)) == NULL) {
    snprintf(syslog_message,message_size,"cut_im_black_swath(): %s", 
	       gMessages[E_CS_BADMALLOC]);
    wrap_syslog (LOG_CRIT,"%s",syslog_message); 
    return 0;
  }
  
  /* read in and analyze image data for a black swath */  
  { 
    int retval = 0;
    retval = read_black_image( data, image, printer, byte_width,
			       vpos_shift, maxlines, swath_limits, last_line); 
    if(retval != 2) {
      free (data[0]);
      return retval;
    }
  }
  /* analysis of the image is now complete */    
  
  /* width of the swath in bytes */
  /* change sweep params to ensure at least 4 bytes sweep width */
  swath_limits->right += 3;
  swath_limits->left-- ;
  
  /* calculate vertical position */
  sweep_data->vertical_pos = swath_limits->last_line +  
    swath_limits->post_blanklines; 
  /* subtract that 600 dot adjustment here  */
  sweep_data->vertical_pos -= vpos_shift;

  /* fix sweep direction (right_to_left or left_to_right) */
  sweep_data->direction = unknown;  
  /* always make left_to_right black sweeps if gUnimode is true */
  /* also do this  to avoid striping in multipass color print */
  if (gUnimode  || (gColorMode && !(gMaxPass %2) && !gCalibrate ))
    sweep_data->direction = left_to_right;
  else if (gCalibrate) {
    /* use legacy direction choosing method from pnm2ppa-1.04 */
    if (prev_color_dir == unknown  &&
	prev_black_dir == unknown )
      sweep_data->direction = left_to_right;
    else if (prev_black_vpos > prev_color_vpos || 
	 prev_color_vpos > sweep_data->vertical_pos ){
      if (prev_black_dir == left_to_right)
	sweep_data->direction = right_to_left;
      else
	sweep_data->direction = left_to_right;
    } else {
      if (prev_color_dir == left_to_right)
	sweep_data->direction = right_to_left;
      else
	sweep_data->direction = left_to_right;
    }
  } else {
    int opposite_dir = unknown;
    if (current_swath) {
      if (prev_color_dir != unknown && 
	  prev_color_vpos < sweep_data->vertical_pos
	  && current_swath->vertical_pos <= prev_color_vpos ) 
	opposite_dir = prev_color_dir;
      else
	opposite_dir = current_swath->direction;
    } else {
      if (prev_black_dir != unknown)
	opposite_dir = prev_black_dir;
      else
	opposite_dir = right_to_left;
    }
    if (opposite_dir == right_to_left)
      sweep_data->direction = left_to_right;
    else if (opposite_dir == left_to_right)
      sweep_data->direction = right_to_left;
  }
  
  assert (sweep_data->direction != unknown);
  
  sweep_data->in_color = false;    
  
  /* 
     create sweep image_data  
     "width" is horizontal swath width in bytes;
     will add 12 null bytes on each side of black swath 
  */
  width = swath_limits->right - swath_limits->left + 24; 
  
  if ((ppa = malloc (width * swath_limits->numlines)) == NULL) {
    snprintf(syslog_message,message_size,"cut_im_black_swath(): %s", 
	     gMessages[E_CS_BADPPAMALLOC]);
    wrap_syslog (LOG_CRIT,"%s",syslog_message); 
    free (data[0]);
    return 0;
  }    
  place = ppa;

  if (sweep_data->direction == right_to_left)	/* right-to-left */
    maxplace = r2l_black_sweep(place, data[0], byte_width,
			       swath_limits);       
  else			/* sweep_data->direction == left_to_right */
    maxplace = l2r_black_sweep(place, data[0],  byte_width,
			       swath_limits);     
  sweep_data->image_data = ppa;
  sweep_data->data_size = maxplace - ppa;
  sweep_data->first_data_line = swath_limits->first_line;
  sweep_data->last_data_line = swath_limits->last_line;
  sweep_data->pre_blanklines = swath_limits->pre_blanklines;
  sweep_data->post_blanklines = swath_limits->post_blanklines;
  
  /* done with data */
  free (data[0]);
  
  horizpos = swath_limits->left * 8 ;
  horizpos += (sweep_data->direction == left_to_right) ?  1  : 0;      
  if (sweep_data->direction == right_to_left ) 
    horizpos +=  printer->r2l_bw_offset ;/* correct bidirectional shearing  */
  
  sweep_data->left_margin = horizpos;
  sweep_data->right_margin = horizpos + printer->marg_diff + width  * 8;
  
  
  if (!black_nozzle_data(sweep_data, swath_limits->numlines, printer->marg_diff )) {
    DPRINTF("black_nozzle_data: malloc failed");
    free(sweep_data->image_data);
    return 0;
  }
  prev_black_vpos = sweep_data->vertical_pos;
  prev_black_dir = sweep_data->direction;
  
  DPRINTF ("cut_im_black_swath: created swath, return 2\n");
  return 2;
}

int
read_black_image( unsigned char * data[], image_t *image, 
		  ppaPrinter_t *printer, int byte_width,
		  int vpos_shift, int maxlines,
		  ppaSwathLimits_t *swath_limits,
		  int last_line)
{
  BOOLEAN  got_nonblank = false, after_color = false, ignore_color = false;
  int  i, j;
  int max_vpos, min_vpos;
  int left, right;  
  int numlines = 0;
  BOOLEAN blackline = true;
  int non_blanklines = 0,  pre_blanklines = 0,  post_blanklines = 0;
  ppaPageLimits_t *page_limit = &BlackPageLimits;

  min_vpos = image->blackCurLine - vpos_shift;
  max_vpos = BlackPageLimits.bottom - vpos_shift;

  /* test to see whether color swaths can be ignored */
  if(!gColorMode ||
     !(waiting_color ||
       (current_swath && current_swath->in_color ) || 
       image->colorCurLine < ColorPageLimits.bottom ) ) 
    ignore_color = true;

  /* eat all lines that are below the lower margin */
  if (image->blackCurLine >= page_limit->bottom ){
    while (image->blackCurLine < image->height)
      if (!im_black_readline (image, data, 0)){
	snprintf(syslog_message,message_size,"read_black_image(): %s", 
		 gMessages[E_CS_BADBOTMARG]);
	wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	
	return 0;
      }
    return 1;
  }
  
  left = page_limit->right - 1;
  right = page_limit->left;
  
  /* eat all beginning blank lines and then up to maxlines or lower margin */
  
  while (( image->blackCurLine  + pre_blanklines  < page_limit->bottom)
	 && (image->blackCurLine <  last_line)
	 && (numlines + pre_blanklines + post_blanklines   < maxlines)
	 && image->blackCurLine < max_vpos + vpos_shift
	 && (!gColorMode || got_nonblank
	       || ((image->buflines < MAXBUFFLINES)
		   || (image->buftype == bitmap)))) {
    if (!im_black_readline (image, data, byte_width * numlines)) {
      snprintf(syslog_message,message_size,"read_black_image(): %s", 
	       gMessages[E_CS_BADNEXTLINE]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
      return 0;
    }
    blackline = image->LineType[image->blackCurLine - 1] & BLACKLINE;
    if (!got_nonblank) {
      if (blackline){
	for (i = page_limit->left ; i < page_limit->right; i++) {
	  if (data[0][i]){
	    left = i;
	    got_nonblank = true;
	    right = i;
	    for (j = page_limit->right - 1; j > left ; j--){
	      if (data[0][j]){
		right = j;
		break;
	    }
	    }
	    break;
	  }
	}
      }
      if(got_nonblank) {
	/* begin a new swath;, nonblank pixels occur in
	 * bytes "left" through "right", inclusive,
	 * where
	 * page_limit->left <= left <= right < page_limit->right.
	 * This range will be expanded if necessary
	 * as the swath is constructed 
	 */
	DPRINTF("read_black_image: begin swath, line %d\n",
		image->blackCurLine ) ;
	
	numlines = 1;
	non_blanklines = 1;
	swath_limits->first_line = image->blackCurLine;
	min_vpos = image->blackCurLine - vpos_shift;
	max_vpos = min_vpos + maxlines - 1 ;

	/* 
	   fix to avoid flashing lights on HP820C: 
	   Empirical observation shows that on HP820C,
	   a new swath cannot have a vertical_position that differs from
	   the previous swath that was printed by 1, 2 or 3 lines. 
	   This problem does not occur on HP71x/72xC.
	*/
	if(current_swath &&  min_vpos < current_swath->vertical_pos + 4)
	  min_vpos = current_swath->vertical_pos + 4;

	if(ignore_color) {
	  if (max_vpos >= BlackPageLimits.bottom - vpos_shift + 20)
	    max_vpos = BlackPageLimits.bottom - vpos_shift + 20;
	} else if( waiting_color) {
	  if (max_vpos >= waiting_color->vertical_pos
	      && waiting_color->vertical_pos >= min_vpos) {
	    /*
	      design this swath to have the same vertical_pos
	      as the waiting color swath, and print immediately after it.
	    */
	    pre_blanklines = max_vpos - waiting_color->vertical_pos;
	    max_vpos = waiting_color->vertical_pos;
	    min_vpos = max_vpos;
	    after_color = true;
	  } else if (max_vpos < waiting_color->vertical_pos) {
	    /*
	      design this swath to print before the
	      waiting color swath, and to not be overprinted by
	      the waiting color swath.  
	    */
	    if (max_vpos > waiting_color->vertical_pos - 4)
	      max_vpos =  waiting_color->vertical_pos - 4;
	    assert (max_vpos >= min_vpos);
	    if (last_line > waiting_color->first_data_line  - 1 )
	      last_line = waiting_color->first_data_line  - 1;

	    if(max_vpos + vpos_shift + 1 - maxlines < 
	       swath_limits->first_line) {
	      pre_blanklines = maxlines + swath_limits->first_line  
		- max_vpos - vpos_shift - 1;
	    }
	  } else if (waiting_color->vertical_pos < min_vpos) {
	    /*
	      design this swath to print with vpos
	      at least 4 greater than that of  the waiting color swath.
	    */
	    if (min_vpos < waiting_color->vertical_pos + 4)
	      min_vpos = waiting_color->vertical_pos + 4;
	    assert (max_vpos >= min_vpos);
	  }
	}
      }
    } else {
      int newleft = left, newright = right;
      if (blackline) {
	/* find left-most nonblank */
	for (i = page_limit->left ; i < left; i++){
	  if (data[0][byte_width * numlines + i]){
	    newleft = i;
	    break;
	}
	}
	/* find right-most nonblank */
	for (j = page_limit->right - 1; j > right; j--){
	  if (data[0][byte_width * numlines + j]){
	    newright = j;
	    break;
	  }
	}
      }
      numlines++;
      
      if (newright < newleft){
	  DPRINTF ("Code error! newleft=%d, newright=%d, left=%d, right=%d\n",
		   newleft, newright, left, right);
	  return 0;
      }
      
      /* if the next line might push us over the buffer size, stop here! */
      /* ignore this test for the 720 right now.  Will add better */
      /* size-guessing for compressed data in the near future! */
      if (numlines % 2 == 1 && printer->version != HP7X0) {
	int l = newleft, r = newright, w;
	
	l--;
	r += 3;
	w = r - l;
	
	if ((w + 24) * numlines > printer->bufsize){
	  numlines--;
	  im_unreadline (image, data[0] + byte_width * numlines);
		  break;
	} else {
	  left = newleft;
	  right = newright;
	  if (blackline) 
	    non_blanklines = numlines;
	}
      } else {
	left = newleft;
	right = newright;
	if (blackline) 
	  non_blanklines = numlines;
      }
    }
  }
  
  if ((gColorMode) && (image->buflines >= MAXBUFFLINES)
      && (image->buftype == color)){
      DPRINTF ("read_black_image: exceeding buffer size: buflines=%d, MAX=%d\n",
	       image->buflines, MAXBUFFLINES);
      if ((!got_nonblank)){
	return 3;
      }
  }
  
  if ((!got_nonblank)) {
      /* eat all lines that are below the lower margin */
      if (image->blackCurLine >= page_limit->bottom ){
	while (image->blackCurLine < image->height)
	  if (!im_black_readline (image, data, 0)){
	    snprintf(syslog_message,message_size,"read_black_image(): %s", 
		     gMessages[E_CS_BADBOTMARG]);
	    wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	    
	    return 0;
	  }
	return 1;
      }   
      if(image->blackCurLine == last_line) 
	return 3;
      return 0;		
      /* error, since didn't get to lower margin, yet blank */
  }
  
  /* 
     remove any blank lines at end of swath;
  */
  
  swath_limits->last_line = image->blackCurLine;
  swath_limits->last_line += (non_blanklines - numlines);
  /* calculate number of blank lines added at top and bottom of swath */
  numlines = non_blanklines;

  assert (swath_limits->last_line - vpos_shift <= max_vpos);
  if (swath_limits->last_line - vpos_shift > min_vpos)
    min_vpos = swath_limits->last_line - vpos_shift;
  post_blanklines = min_vpos - swath_limits->last_line + vpos_shift;
  numlines += post_blanklines;
  pre_blanklines = numlines % 2;
  numlines += pre_blanklines;


  DPRINTF("read_black_image: end swath, line %d numlines=%d= "
	  " (%d,%d,%d) left=%d right=%d\n",
	  image->blackCurLine, numlines, 
	  pre_blanklines, non_blanklines, post_blanklines,	  
	  left, right) ;

  
  assert(pre_blanklines >= 0 
	 && post_blanklines >= 0
	 && non_blanklines >= 0
	 && numlines <= maxlines
	 && numlines % 2 == 0);

  swath_limits->left = left;
  swath_limits->right = right; 
  swath_limits->numlines = numlines;
  swath_limits->non_blanklines = non_blanklines; 
  swath_limits->pre_blanklines = pre_blanklines; 
  swath_limits->post_blanklines = post_blanklines; 
  
  return 2;
}

int
black_nozzle_data(ppaSweepData_t *sweep_data, int numlines, int margin_diff )
{
  int i;
  ppaNozzleData_t nozzles[2];
  for (i = 0; i < 2; i++) {
    nozzles[i].DPI = 600;
    nozzles[i].pins_used_d2 = numlines / 2;
    nozzles[i].unused_pins_p1 = 301 - numlines;
    nozzles[i].first_pin = 1;
    if (i == 0) {
      nozzles[i].left_margin = sweep_data->left_margin + margin_diff;
      nozzles[i].right_margin = sweep_data->right_margin;
      if (sweep_data->direction == right_to_left)
	nozzles[i].nozzle_delay = 0;
      else
	nozzles[i].nozzle_delay = 6;	
    } else {
      nozzles[i].left_margin = sweep_data->left_margin;
      nozzles[i].right_margin = sweep_data->right_margin - margin_diff;
      if (sweep_data->direction == right_to_left)
	nozzles[i].nozzle_delay = 2;
      else
	nozzles[i].nozzle_delay = 0;
    }
  }
  
  sweep_data->nozzle_data_size = 2;
  sweep_data->nozzle_data = malloc (sizeof (nozzles));
  if (sweep_data->nozzle_data == NULL)
    return 0;
  memcpy (sweep_data->nozzle_data, nozzles, sizeof (nozzles));
  return 2;
}

unsigned char *
r2l_black_sweep(unsigned char *place, unsigned char *data,
		int data_width, ppaSwathLimits_t * swath_limits)
{
  int i, j , width, evenlines, oddlines;

  int left = swath_limits->left;
  int right = swath_limits->right;
  int non_blanklines = swath_limits->non_blanklines;
  int pre_blanklines = swath_limits->pre_blanklines;
  int post_blanklines = swath_limits->post_blanklines;
  int even_non_blanklines, even_pre_blanklines, even_post_blanklines;
  int odd_non_blanklines, odd_pre_blanklines, odd_post_blanklines;
  int start_even, start_odd;
  ppaPageLimits_t * page_limit = &BlackPageLimits;

#ifdef DEBUG
  int data_size = data_width * swath_limits->non_blanklines;
#endif

  even_pre_blanklines   = odd_pre_blanklines  = pre_blanklines  / 2 ;
  even_non_blanklines   = odd_non_blanklines  = non_blanklines  / 2 ;
  even_post_blanklines  = odd_post_blanklines = post_blanklines / 2 ;

  start_even= 0;
  start_odd = 1;
  
  if ( (pre_blanklines %2 == 0) &&
       !(non_blanklines % 2 == 0) && 
       !(post_blanklines % 2 == 0) ){ 
    even_non_blanklines++ ;
    odd_post_blanklines++;
    start_even= 0;
    start_odd = 1;
  }
  if ( !(pre_blanklines %2 == 0) &&
       !(non_blanklines % 2 == 0) && 
       (post_blanklines % 2 == 0)){ 
	even_pre_blanklines++;
	odd_non_blanklines++;
	start_even= 1;
	start_odd = 0;
  }
  if ( !(pre_blanklines %2 == 0) &&
       (non_blanklines % 2 == 0) && 
       !(post_blanklines % 2 == 0)){ 
    even_pre_blanklines++;
    odd_post_blanklines++;
    start_even= 1;
    start_odd = 0;
  }
  
  width = right - left ;
  evenlines = even_pre_blanklines + even_non_blanklines + even_post_blanklines;
  oddlines = odd_pre_blanklines + odd_non_blanklines + odd_post_blanklines;
  
  assert ( oddlines == evenlines );
  
  /* place 0's in the first 12 columns */
  memset (place, 0, evenlines * 12);
  place += evenlines * 12;
  
  for (i = width + 11; i >= 0; i--) {
    int x ;
    if (i >= 12){
      x = i - 12;
      if( left + x  < page_limit->left || left + x >= page_limit->right ) 
	{
	  /* never print data outside the limits */
	  memset (place, 0, evenlines);
	  place += evenlines;
	} else {
	  if ( even_pre_blanklines > 0) {
	    /* first pre_blanklines lines are blank */
	    memset (place, 0, even_pre_blanklines) ;
	    place += even_pre_blanklines ;   
	  }
	  for (j = 0; j < even_non_blanklines; j++) {
	    int index = ((j * 2) + start_even ) * data_width + left + x;
#ifdef DEBUG
	    if (!( index < data_size  && index >= 0) ){
	      DPRINTF("index=%d, datasize= %d =  %d x %d\n",
		      index, data_size, data_width, swath_limits->non_blanklines);
	    }
	    assert ( index  < data_size   && index >= 0 );
#endif
	    *place++ = data[index];
	  }
	  if ( even_post_blanklines > 0 ) {
	    /* last post_blanklines lines are blank */
	    memset (place, 0, even_post_blanklines) ;
	    place += even_post_blanklines ;
	  }
	}
    } else {
      memset (place, 0, evenlines);
      place += evenlines;
    }
    
    if (i < width) {
      x = i ;
      if( left + x  < page_limit->left || left + x >= page_limit->right ) {
	/* never print data outside the limits */
	memset (place, 0, evenlines);
	place += evenlines;
      } else {
	if ( odd_pre_blanklines > 0) {
	  /* first pre_blanklines lines are blank */
	  memset (place, 0, odd_pre_blanklines) ;
	  place += odd_pre_blanklines ;   
	}
	for (j = 0; j < odd_non_blanklines; j++) {
	  int index = ((j * 2) + start_odd ) * data_width + left + x;
#ifdef DEBUG
	  if (!( index < data_size  && index >= 0) ){
	    DPRINTF("index=%d, datasize= %d =  %d x %d\n",
		    index, data_size, data_width, swath_limits->non_blanklines);
	  }
	  assert ( index  < data_size   && index >= 0 );
#endif
	  *place++ = data[index];
	}
	if ( odd_post_blanklines > 0 ){
	  /* last post_blanklines lines are blank */
	  memset (place, 0, odd_post_blanklines) ;
	  place += odd_post_blanklines ;
	}
      }
    } else {
      memset (place, 0, evenlines);
      place += evenlines;
    }
  }
  /* place 0's in the last 12 columns */
  memset (place, 0, evenlines * 12);
  place += evenlines * 12;
  return place;
}

unsigned char *
l2r_black_sweep(unsigned char *place, unsigned char *data, 
		int data_width, ppaSwathLimits_t * swath_limits)
{
  int i, j , width, evenlines, oddlines;
  
  int left = swath_limits->left;
  int right = swath_limits->right;
  int non_blanklines = swath_limits->non_blanklines;
  int pre_blanklines = swath_limits->pre_blanklines;
  int post_blanklines = swath_limits->post_blanklines;
  int even_non_blanklines, even_pre_blanklines, even_post_blanklines;
  int odd_non_blanklines, odd_pre_blanklines, odd_post_blanklines;
  int start_even, start_odd;
  ppaPageLimits_t * page_limit = &BlackPageLimits;

#ifdef DEBUG 
  int data_size = data_width * swath_limits->non_blanklines; 
#endif

  even_pre_blanklines   = odd_pre_blanklines  = pre_blanklines  / 2 ;
  even_non_blanklines   = odd_non_blanklines  = non_blanklines  / 2 ;
  even_post_blanklines  = odd_post_blanklines = post_blanklines / 2 ;

  start_even= 0;
  start_odd = 1;
  
  if ( (pre_blanklines %2 == 0) &&
       !(non_blanklines % 2 == 0) && 
       !(post_blanklines % 2 == 0))  { 
    even_non_blanklines++ ;
    odd_post_blanklines++;
    start_even= 0;
    start_odd = 1;
  }
  if ( !(pre_blanklines %2 == 0) &&
       !(non_blanklines % 2 == 0) && 
       (post_blanklines % 2 == 0)) { 
    even_pre_blanklines++;
    odd_non_blanklines++;
    start_even= 1;
    start_odd = 0;
  }
  if ( !(pre_blanklines %2 == 0) &&
       (non_blanklines % 2 == 0) && 
       !(post_blanklines % 2 == 0)) { 
    even_pre_blanklines++;
    odd_post_blanklines++;
    start_even= 1;
    start_odd = 0;
  }
  
  width = right - left ;
  evenlines = even_pre_blanklines + even_non_blanklines + even_post_blanklines;
  oddlines = odd_pre_blanklines + odd_non_blanklines + odd_post_blanklines;
  
  assert ( oddlines == evenlines );
  
  /* place 0's in the first 12 columns */
  memset (place, 0, evenlines * 12);
  place += evenlines * 12;
  
  for (i = 0; i < width + 12; i++) {
    int x;
    if (i < width) {
      x = i;
      if( left + x  < page_limit->left || left + x >= page_limit->right ) {
	/* never print data outside the limits */
	memset (place, 0, evenlines);
	place += evenlines;
      } else {
	if ( odd_pre_blanklines > 0) {
	  /* first pre_blanklines lines are blank */
	  memset (place, 0, odd_pre_blanklines) ;
	  place += odd_pre_blanklines ;   
	}
	for (j = 0; j < odd_non_blanklines; j++){
	  int index = ((j * 2) + start_odd) * data_width + left + x;
#ifdef DEBUG
	  if (!( index < data_size  && index >= 0) ){
	    DPRINTF("index=%d, datasize= %d =  %d x %d\n",
		    index, data_size, data_width, swath_limits->non_blanklines);
	  }
	  assert ( index  < data_size   && index >= 0 );
#endif
	  *place++ = data[index];
	}
	if ( odd_post_blanklines > 0 ){
	  /* last post_blanklines lines are blank */
	  memset (place, 0, odd_post_blanklines) ;
	  place += odd_post_blanklines ;
	}
      }
    } else {
      memset (place, 0, evenlines);
      place += evenlines;
    }
    
    if (i >= 12){
      x = i - 12;
      if( left + x  < page_limit->left || left + x >= page_limit->right ) {
	/* never print data outside the limits */
	memset (place, 0, evenlines);
	place += evenlines;
      } else {
	if ( even_pre_blanklines > 0) {
	  /* first pre_blanklines lines are blank */
	  memset (place, 0, even_pre_blanklines) ;
	  place += even_pre_blanklines ;   
	}
	for (j = 0; j < even_non_blanklines; j++) {
	  int index = ((j * 2) + start_even) * data_width + left + x;
#ifdef DEBUG
	  if (!( index < data_size  && index >= 0) ){
	    DPRINTF("index=%d, datasize= %d =  %d x %d\n",
		    index, data_size, data_width, swath_limits->non_blanklines);
	  }
	  assert ( index  < data_size   && index >= 0 );
#endif
	  *place++ = data[index];
	}
	if ( even_post_blanklines > 0 ){
	  /* last post_blanklines lines are blank */
	  memset (place, 0, even_post_blanklines) ;
	  place += even_post_blanklines ;
	}
      }
    } else {
      memset (place, 0, evenlines);
      place += evenlines;
    }
  }
  /* place 0's in the last 12 columns */
  memset (place, 0, evenlines * 12);
  place += evenlines * 12;
  
  return place;
}

int
cut_im_color_swath (image_t *image, ppaPrinter_t *printer,
		    ppaSweepData_t *sweep_data[4])
{
  unsigned char *data[4], *ppa, *place, *maxplace;
  int j, k;
  int width, byte_width =(image->width / 2 + 7) / 8;
  int vpos_shift=600;   /* "vertical position" = line number - vpos_shift */
  int sweep_dir, sweep_vpos;
  int horizpos;
  int maxlines = 64;   /* fixed by design of print head */
  ppaSwathLimits_t Swath_Limits;
  ppaSwathLimits_t *swath_limits = &Swath_Limits;
  ppaPageLimits_t *page_limit = &ColorPageLimits;
  
  place = NULL;
  ppa = NULL;
  maxplace = NULL;
  
  /* safeguard against the user freeing these */
  for (k = 0; k < gMaxPass; k++) {
    sweep_data[k]->nozzle_data = NULL;
    sweep_data[k]->image_data = NULL;
  }
  
  DPRINTF ("byte_width * 3 * maxlines = %d\n", byte_width * 3 * maxlines);
  for (k = 0; k < gMaxPass; k++) {
    if ((data[k] = malloc ( byte_width * 3 * maxlines + 8)) == NULL){
      snprintf(syslog_message,message_size,"cut_im_color_swath(): %s", 
	       gMessages[E_CS_BADMALLOC]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
      return 0;
    }
  }
  /* read the image data from the input file */
  {
    int retval;
    retval = read_color_image(data, image, printer,  byte_width,
			      vpos_shift,  maxlines, swath_limits); 
    if (retval != 2) {
      for (k = 0; k < gMaxPass; k++)
	free (data[k]);
      return retval;
    }
    
  }
  /* analysis of the image is now complete */
  
  /* now determine vertical position of swath */
  sweep_vpos = swath_limits->last_line + 2 * swath_limits->post_blanklines; 
  sweep_vpos += printer->ColBwOffsY ;
  /* subtract that 600 dot adjustment here  */
  sweep_vpos -= vpos_shift;
  
  /* printing in the leftmost and rightmost bytes of the color
   * swath leads to problems, so expand the swath width to avoid
   * using these locations.  Nothing will ever get printed in these
   * expanded  ends of the swath.   (They are either whitespace,
   * or outside the limits, where data is rejected below)
   */
  swath_limits->left -= 2;
  swath_limits->right += 3;
  /* enforce a minimum swath width of 8 bytes */
  if ( swath_limits->right - swath_limits->left < 8 )  {
    if  (swath_limits->right > page_limit->left + 8 )
      swath_limits->left = swath_limits->right - 8 ;
    else
      swath_limits->right = swath_limits->left + 8 ;
  }
  
  /* fix sweep direction (right_to_left or left_to_right) */
  sweep_dir = unknown;
  if (gCalibrate && !gUnimode) {
    /* legacy pnm2ppa-1.04 method for choosing sweep direction */
    if (prev_color_dir == unknown  &&
	prev_black_dir == unknown )
      sweep_dir = left_to_right;
    else if ( prev_black_vpos > prev_color_vpos && 
	      prev_black_vpos < sweep_vpos ) {
      if (prev_black_dir == left_to_right)
	sweep_dir = right_to_left;
      else
	sweep_dir = left_to_right;
    } else {
      if (prev_color_dir == left_to_right)
	  sweep_dir = right_to_left;
      else
	sweep_dir = left_to_right;
    }
  } else if (!gUnimode && gMaxPass % 2){  
    int opposite_dir = unknown;
    /* the correct sweep direction is not yet known, and  must be 
       guessed from sweep_vpos and the previous history 
       (only for gMaxPass == 1) */
    
    if (current_swath){
      if (prev_black_dir != unknown && 
	  prev_black_vpos < sweep_vpos 
	  &&  current_swath->vertical_pos < prev_black_vpos ) 
	opposite_dir = prev_black_dir;
      else
	opposite_dir = current_swath->direction;
    } else {
      /* must be at beginning of page, when prev is still NULL */
      if (prev_black_dir != unknown )
	opposite_dir = prev_black_dir;
      else   /* this is the first time data was found */
	opposite_dir= right_to_left;
    }
    if (opposite_dir == left_to_right)
      sweep_dir = right_to_left;
    else if (opposite_dir == right_to_left)
      sweep_dir = left_to_right;
  }
  
  /* create sweep_data for gMaxPass sweeps of the color print head */

  for (k = 0; k < gMaxPass; k++) {
    sweep_data[k]->direction = sweep_dir;
    if(!gUnimode && !(gMaxPass %2 ) && !gCalibrate) {
      if( k %2 )
	sweep_data[k]->direction = right_to_left;
      else
	sweep_data[k]->direction = left_to_right;
	}
    if (gUnimode){
      if (gPixmapMode)
	sweep_data[k]->direction = left_to_right;
      else
	sweep_data[k]->direction = right_to_left;
    }
    
    assert ( sweep_data[k]->direction != unknown );
    sweep_data[k]->vertical_pos = sweep_vpos;
    sweep_data[k]->in_color = true;	
    sweep_data[k]->image_data = NULL;
    sweep_data[k]->first_data_line = swath_limits->first_line;
    sweep_data[k]->last_data_line = swath_limits->last_line;
    sweep_data[k]->pre_blanklines = swath_limits->pre_blanklines;
    sweep_data[k]->post_blanklines = swath_limits->post_blanklines;
    
    /* create sweep image_data */
    width = swath_limits->right - swath_limits->left + 3;
    if ((ppa = malloc ( width * swath_limits->numlines * 3)) == NULL){
      snprintf(syslog_message,message_size,"cut_im_color_swath(): %s", 
	       gMessages[E_CS_BADPPAMALLOC]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
      
      for (j = k ; j < gMaxPass; j++)
	free (data[j]);   /* free unprocessed data */
      for (j = 0; j < k; j++){
	free (sweep_data[j]->image_data);
	free (sweep_data[j]->nozzle_data);
      }
      return 0;
    }
    place = ppa;
    if (sweep_data[k]->direction == right_to_left)
      maxplace  = r2l_color_sweep(place, data[k], byte_width,
				  swath_limits);    
    else   /* sweep direction is left-to-right  */
      maxplace = l2r_color_sweep(place, data[k], byte_width,
				 swath_limits);     
    free (data[k]);          /* finished with data[k] */
    sweep_data[k]->image_data = ppa;
    sweep_data[k]->data_size = maxplace - ppa;
    
    /* 
       left and right limits of sweep  
       (looks like some tweaks have been made!)
    */
    
    horizpos = (swath_limits->left - 7 )  * 8 * 2;
    horizpos +=  printer->ColBwOffsX - 600;
    horizpos += (sweep_data[k]->direction == left_to_right) ? 2 : 12 ; 
    
    /*correct bidirectional shearing*/    
    if (sweep_data[k]->direction == right_to_left ) 
      horizpos +=   printer->r2l_col_offset  ;  
    
    sweep_data[k]->left_margin = horizpos;
    sweep_data[k]->right_margin = horizpos + 0x74 +  (width - 2 ) * 2 * 8;
    /* 0x74 used to be "printer->color_marg_diff" */
    
    DPRINTF("sweep data: left_margin = %d, right margin = %d\n",
	    sweep_data[k]->left_margin, sweep_data[k]->right_margin);
    
    if (!color_nozzle_data(sweep_data[k], swath_limits->numlines)){
      DPRINTF("color_nozzle_data: malloc failed");
      for (j=k; j < gMaxPass; j++)
	free(data[j]);
      for (j=0; j <= k; j++)
	free(sweep_data[k]->image_data);
      for (j = 0; j < k; j++)
	free(sweep_data[j]->nozzle_data);
      return 0;
    }
    prev_color_vpos = sweep_data[k]->vertical_pos;
    prev_color_dir = sweep_data[k]->direction;
  }
  DPRINTF ("cut_im_color_swath: created swath, return 2\n");
  return 2;
}


int
read_color_image( unsigned char *data[], image_t *image,
		  ppaPrinter_t *printer, int byte_width,
		  int vpos_shift, int maxlines,
		  ppaSwathLimits_t *swath_limits)
{
  int  i, j;
  int numlines;
  int left, right;
  int non_blanklines = 0, pre_blanklines = 0, post_blanklines = 0 ;
  int  min_vpos;
  BOOLEAN got_nonblank;
  ppaPageLimits_t *page_limit = &ColorPageLimits;
  BOOLEAN colorline = true;;

  /*
    Flashing lights happen on the HP820 if difference of consecutive
    vertical_pos of (different ink ?) sweeps 
    is  -3, -2, -1, 1 , 2 or 3! 
  */

  /* are there any circumstances which require that
     this swath has the same vertical_pos as current_swath?
     assume not, for the moment.
  */
  min_vpos = image->colorCurLine - vpos_shift + printer->ColBwOffsY;
  if (current_swath && !current_swath->in_color ) {
    if(min_vpos < current_swath->vertical_pos + 4)
      min_vpos = current_swath->vertical_pos + 4;
  }
  if (waiting_black) {
    if(min_vpos < waiting_black->vertical_pos + 4)
      min_vpos = waiting_black->vertical_pos + 4;
  }

  
  /* eat all lines that are below the lower margin */
  if (image->colorCurLine >= page_limit->bottom ) {
    while (image->colorCurLine < image->height)
      if (!im_color_readline (image, data, 0)){
	snprintf(syslog_message,message_size,"read_color_image(): %s", 
		 gMessages[E_CS_BADBOTMARG]);
	wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	return 0;
      }
    DPRINTF (" read_color_image: return 1 on line %d\n", __LINE__);
    return 1;
  }
  
  
  left = page_limit->right - 1; 
  right = page_limit->left;
  
  got_nonblank = false;
  numlines = 0;
  pre_blanklines = 0;
  post_blanklines = 0;
  /* eat all beginning blank lines and then up to maxlines or lower margin */
  while (
	 ( image->colorCurLine + (2* pre_blanklines) < page_limit->bottom ) &&
	 ( (numlines + pre_blanklines + post_blanklines) < maxlines)
	 && (got_nonblank ||
	     ((image->buflines < MAXBUFFLINES) || (image->buftype == color)))){

    if (!im_color_readline (image, data, byte_width * 3 * numlines)) {
      snprintf(syslog_message,message_size,"read_color_image(): %s", 
	       gMessages[E_CS_BADNEXTLINE]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
      return 0;
    }
    colorline  = image->LineType[image->colorCurLine - 2] & COLORLINE;
    if (!got_nonblank){
      if (colorline) {
	for (i = page_limit->left * 3; i < page_limit->right * 3; i++){
	  if ((data[gMaxPass - 1][i])){
	    left = i / 3;
	    got_nonblank = true;
	    right = left;
	    for (j = page_limit->right * 3 - 1; j > left * 3 + 2; j--) {
	    if ((data[gMaxPass - 1][j])){
	      right = j / 3;
	      break;
	    }
	    }
	  break;
	  }
	}
      }
      if(got_nonblank) {
	/* begin a new swath;, nonblank pixels occur in
	 * bytes "left" through "right", inclusive,
	 * where
	 * page_limit->left <= left <= right < page_limit->right.
	 * This range will be expanded if necessary
	 * as the swath is constructed 
	 */
	DPRINTF("read_color_image: begin swath, line %d\n",
		image->colorCurLine ) ;
	
	numlines = 1;
	non_blanklines = 1;
	swath_limits->first_line = image->colorCurLine;
	/* We assume this is true! */
	assert (image->colorCurLine - vpos_shift + printer->ColBwOffsY
		+ -1 + ( 2 * maxlines)  >= min_vpos);
      }
    } else {
      /* find left-most nonblank */
      if (colorline) {
	for (i = page_limit->left * 3; i < left * 3; i++) {
	  if ((data[gMaxPass - 1][byte_width * 3 * numlines + i])){
	    left = i / 3;
	    break;
	  }
	}
	/* find right-most nonblank */
	for (j =  page_limit->right * 3 - 1 ;  j > right * 3 + 2 ; j--){
	  if ((data[gMaxPass - 1][byte_width * 3 * numlines + j])){
	    right = j / 3;
	    break;
	  }
	}
      }
      numlines++;
      if (colorline)
	non_blanklines = numlines;
    }
  }
  
  if ((image->buflines >= MAXBUFFLINES) && (image->buftype == bitmap)){
    DPRINTF
      ("read_color_image: exceeding buffer size: image.buflines=%d, MAX=%d\n",
       image->buflines, MAXBUFFLINES);
    if ((!got_nonblank))
      return 3;
  }
  
  
  if ((!got_nonblank)){
    /* eat all lines that are below the lower margin */
    if (image->colorCurLine >= page_limit->bottom){
      while (image->colorCurLine < image->height)
	if (!im_color_readline (image, data, 0)){
	  snprintf(syslog_message,message_size,"read_color_image(): %s", 
		   gMessages[E_CS_BADBOTMARG]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	  return 0;
	}
      DPRINTF
	("read_color_image: return 1 on line %d; ccl: %d; height: %d\n",
	 __LINE__, image->colorCurLine, page_limit->bottom );
      return 1;
    }
    return 0;		
    /* error, since didn't get to lower margin, yet blank */
  }
  
  /* 
     remove any blank lines at end of swath;
  */
  
  swath_limits->last_line = image->colorCurLine;
  swath_limits->last_line += 2*(non_blanklines - numlines);
  numlines = non_blanklines;

  post_blanklines = 0;
  if(swath_limits->last_line - vpos_shift + printer->ColBwOffsY <  min_vpos)
    post_blanklines = min_vpos +  vpos_shift - printer->ColBwOffsY 
      - swath_limits->last_line;

  numlines += post_blanklines;
  pre_blanklines = numlines % 2;
  numlines += pre_blanklines;

  DPRINTF("read_color_image: end swath, line %d numlines=%d= "
	  " (%d,%d,%d) left=%d right=%d\n",
	  image->colorCurLine, numlines, 
	  pre_blanklines, non_blanklines, post_blanklines,	  
	  left, right) ;
  
  swath_limits->left = left;
  swath_limits->right = right; 
  swath_limits->numlines = numlines;
  swath_limits->non_blanklines = non_blanklines; 
  swath_limits->pre_blanklines = pre_blanklines; 
  swath_limits->post_blanklines = post_blanklines; 
  
  return 2;
}

int
color_nozzle_data(ppaSweepData_t * sweep_data, int numlines)
{
  int i;
  ppaNozzleData_t nozzles[6];    
  
  /* create nozzle data  for sweep */
  
  for (i = 0; i < 6; i++){
    nozzles[i].DPI = 300;
    nozzles[i].pins_used_d2 = numlines / 2;
    nozzles[i].unused_pins_p1 = 65 - numlines;
    nozzles[i].first_pin = 1;
    if (i == 0){
      nozzles[i].left_margin = sweep_data->left_margin  + 0x74;
      nozzles[i].right_margin = sweep_data->right_margin;
      if (sweep_data->direction == right_to_left)
	nozzles[i].nozzle_delay = 0;
      else
	nozzles[i].nozzle_delay = 0;
    }
    if (i == 1){
      nozzles[i].left_margin = sweep_data->left_margin + 0x64;
      nozzles[i].right_margin = sweep_data->right_margin - 0x10;
      if (sweep_data->direction == right_to_left)
	nozzles[i].nozzle_delay = 0;
      else
	nozzles[i].nozzle_delay = 0;
    }
    if (i == 2){
      nozzles[i].left_margin = sweep_data->left_margin + 0x3A;
      nozzles[i].right_margin = sweep_data->right_margin -0x3A;
      if (sweep_data->direction == right_to_left)
	nozzles[i].nozzle_delay = 0x0A;
      else
	nozzles[i].nozzle_delay = 0x0A;
    }
    if (i == 3){
      nozzles[i].left_margin = sweep_data->left_margin + 0x3A;
      nozzles[i].right_margin = sweep_data->right_margin - 0x3A;
      if (sweep_data->direction == right_to_left)
	nozzles[i].nozzle_delay = 0x0A;
      else
	nozzles[i].nozzle_delay = 0x0A;
      
    }
    if (i == 4){
      nozzles[i].left_margin = sweep_data->left_margin + 0x10;
      nozzles[i].right_margin = sweep_data->right_margin - 0x64;
      if (sweep_data->direction == right_to_left)
	nozzles[i].nozzle_delay = 0x04;
      else
	nozzles[i].nozzle_delay = 0x04;
      
    }
    if (i == 5){
      nozzles[i].left_margin = sweep_data->left_margin;
      nozzles[i].right_margin = sweep_data->right_margin - 0x74;
      if (sweep_data->direction == right_to_left)
	nozzles[i].nozzle_delay = 0x04;
      else
	nozzles[i].nozzle_delay = 0x04;
      
    }
  }
  
  sweep_data->nozzle_data_size = 6;
  sweep_data->nozzle_data = malloc (sizeof (nozzles));
  if (sweep_data->nozzle_data == NULL)
    return 0;
  memcpy (sweep_data->nozzle_data, nozzles, sizeof (nozzles));
  return 2;
}


#define GET_USABLE(A,i) (!(A[i] & 0x1))
#define GET_COLOR(A,i) ((A[i] >> 1) & 0x3);
#define GET_ODD(A,i) ((A[i] >> 3) & 0x1);
#define GET_X(A,i) ((A[i] >> 4));

#define UNUSABLE 1
#define CYAN (0<<1)
#define MAGENTA (1<<1)
#define YELLOW (2<<1)
#define ODD (1<<3)
#define EVEN (0<<3)
#define XSHIFT 4
#define XPOS(x) ((x)<<XSHIFT)



/* number of "special" columns at left of sweep */
/* different versions for left-to-right and right-to-left sweeps  */
static int Right_size = 42;
static int Right_l2r[] = { 
  UNUSABLE,
  UNUSABLE,
  UNUSABLE,
  UNUSABLE,
  CYAN | EVEN | XPOS (0),
  CYAN | ODD | XPOS (0),
  CYAN | EVEN | XPOS (1),
  UNUSABLE,
  UNUSABLE,
  CYAN | ODD | XPOS (1),
  CYAN | EVEN | XPOS (2),
  UNUSABLE,
  UNUSABLE,
  CYAN | ODD | XPOS (2),
  CYAN | EVEN | XPOS (3),
  UNUSABLE,
  MAGENTA | EVEN | XPOS (0),
  CYAN | ODD | XPOS (3),
  CYAN | EVEN | XPOS (4),
  UNUSABLE,
  MAGENTA | ODD | XPOS (0),
  MAGENTA | EVEN | XPOS (1),
  CYAN | ODD | XPOS (4),
  CYAN | EVEN | XPOS (5),
  UNUSABLE,
  UNUSABLE,
  MAGENTA | ODD | XPOS (1),
  MAGENTA | EVEN | XPOS (2),
  CYAN | ODD | XPOS (5),
  CYAN | EVEN | XPOS (6),
  UNUSABLE,
  UNUSABLE,
  MAGENTA | ODD | XPOS (2),
  MAGENTA | EVEN | XPOS (3),
  CYAN | ODD | XPOS (6),
  CYAN | EVEN | XPOS (7),
  UNUSABLE,
  YELLOW | EVEN | XPOS (0),
  MAGENTA | ODD | XPOS (3),
  MAGENTA | EVEN | XPOS (4),
  CYAN | ODD | XPOS (7),
  CYAN | EVEN | XPOS (8)
};


/* number of "special" columns at left of sweep */
static int Left_size = 32;
static int Left_l2r[] = { 
  YELLOW | ODD | XPOS (0),
  YELLOW | EVEN | XPOS (0),
  YELLOW | ODD | XPOS (1),
  YELLOW | EVEN | XPOS (1),
  YELLOW | ODD | XPOS (2),
  YELLOW | EVEN | XPOS (2),
  YELLOW | ODD | XPOS (3),
  UNUSABLE,
  MAGENTA | ODD | XPOS (0),
  YELLOW | EVEN | XPOS (3),
  YELLOW | ODD | XPOS (4),
  MAGENTA | EVEN | XPOS (0),
  MAGENTA | ODD | XPOS (1),
  YELLOW | EVEN | XPOS (4),
  YELLOW | ODD | XPOS (5),
  MAGENTA | EVEN | XPOS (1),
  MAGENTA | ODD | XPOS (2),
  YELLOW | EVEN | XPOS (5),
  YELLOW | ODD | XPOS (6),
  UNUSABLE,
  MAGENTA | EVEN | XPOS (2),
  MAGENTA | ODD | XPOS (3),
  YELLOW | EVEN | XPOS (6),
  YELLOW | ODD | XPOS (7),
  UNUSABLE,
  CYAN | ODD | XPOS (0),
  MAGENTA | EVEN | XPOS (3),
  MAGENTA | ODD | XPOS (4),
  YELLOW | EVEN | XPOS (7),
  YELLOW | ODD | XPOS (8),
  CYAN | EVEN | XPOS (0),
  CYAN | ODD | XPOS (1)
};


unsigned char *
l2r_color_sweep(unsigned char *place, unsigned char *data, 
		int data_width, ppaSwathLimits_t * swath_limits)
{
  int i, j , width, evenlines, oddlines;

  int left = swath_limits->left;
  int right = swath_limits->right;
  int non_blanklines = swath_limits->non_blanklines;
  int pre_blanklines = swath_limits->pre_blanklines;
  int post_blanklines = swath_limits->post_blanklines;
  int even_non_blanklines, even_pre_blanklines, even_post_blanklines;
  int odd_non_blanklines, odd_pre_blanklines, odd_post_blanklines;
  int start_even, start_odd;
  ppaPageLimits_t * page_limit = &ColorPageLimits;

#ifdef DEBUG
  int data_size = data_width * 3 * swath_limits->non_blanklines;
#endif

  
  even_pre_blanklines   = odd_pre_blanklines  = pre_blanklines  / 2 ;
  even_non_blanklines   = odd_non_blanklines  = non_blanklines  / 2 ;
  even_post_blanklines  = odd_post_blanklines = post_blanklines / 2 ;
  
  start_even= 0;
  start_odd = 1;
  
  if ( (pre_blanklines %2 == 0) &&
       !(non_blanklines % 2 == 0) && 
       !(post_blanklines % 2 == 0)) { 
    even_non_blanklines++ ;
    odd_post_blanklines++;
    start_even= 0;
    start_odd = 1;
  }
  if ( !(pre_blanklines %2 == 0) &&
       !(non_blanklines % 2 == 0) && 
       (post_blanklines % 2 == 0)){ 
    even_pre_blanklines++;
    odd_non_blanklines++;
    start_even= 1;
    start_odd = 0;
  }
  if ( !(pre_blanklines %2 == 0) &&
       (non_blanklines % 2 == 0) && 
       !(post_blanklines % 2 == 0)){ 
    even_pre_blanklines++;
    odd_post_blanklines++;
    start_even= 1;
    start_odd = 0;
  }
  
  width = right - left ;
  evenlines = even_pre_blanklines + even_non_blanklines + even_post_blanklines;
  oddlines = odd_pre_blanklines + odd_non_blanklines + odd_post_blanklines;
  
  assert ( oddlines == evenlines );
  
  for (i = 0; i < (width + 1) * 6 ; i++)  {
    int color, x, odd, y;
    j = (width + 1) * 6  - 1 - i ;
    if (i <  Left_size){
      if (!GET_USABLE (Left_l2r, i)){
	memset (place, 0x00, evenlines);
	place += evenlines;
	continue;
      }
      color = GET_COLOR (Left_l2r, i);
      odd = GET_ODD (Left_l2r, i);
      x = GET_X (Left_l2r, i);
    } else if ( j < Right_size ){
      if (!GET_USABLE (Right_l2r, j)){
	memset (place, 0x00, evenlines);
	place += evenlines;
	continue;
      }
      color = GET_COLOR (Right_l2r, j);
      odd = GET_ODD (Right_l2r, j);
      x = width  -3 - GET_X (Right_l2r,j);
    } else {
      color = (i / 2 - 15) % 3;
      odd = i % 2;
      x = (i - 24) / 6 - 1 + odd + (color == 0 ? 0 :/* cyan */
				    color == 1 ? 4 :/* magenta */
				    8);	          /* yellow */
    }
    if (x + left < page_limit->left || x + left >= page_limit->right) {
      /* never print data that is outside the limits */
      memset (place, 0x00, evenlines);
      place += evenlines;
    } else {
      int new_pre_blanklines , new_non_blanklines , new_post_blanklines;
      int new_start;
      if (odd) {
	/* odd lines */
	new_pre_blanklines  = odd_pre_blanklines  ;
	new_non_blanklines  = odd_non_blanklines  ;
	new_post_blanklines = odd_post_blanklines ;
	new_start = start_odd;
      } else {
	/* even lines */
	new_pre_blanklines  = even_pre_blanklines  ;
	new_non_blanklines  = even_non_blanklines  ;
	new_post_blanklines = even_post_blanklines ;
	new_start = start_even;
      }
      if ( new_pre_blanklines > 0 )  {
	/* first pre_blanklines lines are blank */
	memset (place, 0x00, new_pre_blanklines ) ;
	place += new_pre_blanklines  ;
      }
      for (y = 0; y < new_non_blanklines; y++) {
	int index = ((y * 2 + new_start) * data_width * 3 +
		 (x + left ) * 3 + color);
#ifdef DEBUG	
	if (!( index < data_size  && index >= 0) ){
	  DPRINTF("index=%d, datasize= %d =  %d x %d\n",
		  index, data_size, data_width, swath_limits->non_blanklines);
	}
	assert ( index < data_size  && index >= 0 );
#endif
	*place++ = data[index];
      }
      if ( new_post_blanklines > 0 ){
	/* last post_blanklines lines are blank */
	memset(place, 0x00, new_post_blanklines) ;
	place += new_post_blanklines ;
      }
    }
  }
  return place;
}

static int Right_r2l[] = { 
  UNUSABLE,
  UNUSABLE,
  UNUSABLE, 
  CYAN | EVEN | XPOS (0),
  UNUSABLE,
  CYAN | EVEN | XPOS (1), 
  CYAN | ODD | XPOS (0),
  UNUSABLE,
  UNUSABLE,
  CYAN | EVEN | XPOS (2),
  CYAN | ODD | XPOS (1),
  UNUSABLE,
  UNUSABLE,
  CYAN | EVEN | XPOS (3),
  CYAN | ODD | XPOS (2),
  MAGENTA | EVEN | XPOS (0),
  UNUSABLE,   
  CYAN | EVEN | XPOS (4),
  CYAN | ODD | XPOS (3),
  UNUSABLE,
  MAGENTA | EVEN | XPOS (1),
  MAGENTA | ODD | XPOS (0),
  CYAN | EVEN | XPOS (5),
  CYAN | ODD | XPOS (4),
  UNUSABLE,
  UNUSABLE,
  MAGENTA | EVEN | XPOS (2),
  MAGENTA | ODD | XPOS (1),
  CYAN | EVEN | XPOS (6),
  CYAN | ODD | XPOS (5),
  UNUSABLE,
  UNUSABLE,
  MAGENTA | EVEN | XPOS (3),
  MAGENTA | ODD | XPOS (2),
  CYAN | EVEN | XPOS (7),
  CYAN | ODD | XPOS (6),
  YELLOW | EVEN | XPOS (0),
  UNUSABLE,
  MAGENTA | EVEN | XPOS (4),
  MAGENTA | ODD | XPOS (3),
  CYAN | EVEN | XPOS (8),
  CYAN | ODD | XPOS (7)
};


/* the final odd yellow swing buffer doesnt fit in right-to-left
 * color sweeps.  Instead of redesigning the structure of  the Left of
 * the sweep (If it works, dont fix it ...)
 * I'll just arrange that no printable data ever uses
 * this last position .. (the leftmost printed dot) (duncan)
 */

static int Left_r2l[] = { 
  UNUSABLE,/*YELLOW | ODD | XPOS (0),   this data doesnt fit,  what to do?*/  
  YELLOW | ODD | XPOS (1),
  YELLOW | EVEN | XPOS (0),
  YELLOW | ODD | XPOS (2),
  YELLOW | EVEN | XPOS (1),
  YELLOW | ODD | XPOS (3),
  YELLOW | EVEN | XPOS (2),
  MAGENTA | ODD | XPOS (0),
  UNUSABLE,
  YELLOW | ODD | XPOS (4),
  YELLOW | EVEN | XPOS (3),
  MAGENTA | ODD | XPOS (1),
  MAGENTA | EVEN | XPOS (0),
  YELLOW | ODD | XPOS (5),
  YELLOW | EVEN | XPOS (4),
  MAGENTA | ODD | XPOS (2),
  MAGENTA | EVEN | XPOS (1),
  YELLOW | ODD | XPOS (6),
  YELLOW | EVEN | XPOS (5),
  UNUSABLE, 
  MAGENTA | ODD | XPOS (3),
  MAGENTA | EVEN | XPOS (2),
  YELLOW | ODD | XPOS (7),
  YELLOW | EVEN | XPOS (6),
  CYAN | ODD | XPOS (0),
  UNUSABLE, 
  MAGENTA | ODD | XPOS (4),
  MAGENTA | EVEN | XPOS (3),
  YELLOW | ODD | XPOS (8),
  YELLOW | EVEN | XPOS (7),
  CYAN | ODD | XPOS (1),
  CYAN | EVEN | XPOS (0)
};


unsigned char *
r2l_color_sweep(unsigned char *place, unsigned char *data, 
		int data_width, ppaSwathLimits_t * swath_limits)
{
  int i, j , width, evenlines, oddlines;

  int left = swath_limits->left;
  int right = swath_limits->right;
  int non_blanklines = swath_limits->non_blanklines;
  int pre_blanklines = swath_limits->pre_blanklines;
  int post_blanklines = swath_limits->post_blanklines;
  int even_non_blanklines, even_pre_blanklines, even_post_blanklines;
  int odd_non_blanklines, odd_pre_blanklines, odd_post_blanklines;
  int start_even, start_odd;
  ppaPageLimits_t * page_limit = &ColorPageLimits;

#ifdef DEBUG
  int data_size = data_width * 3 * swath_limits->non_blanklines;
#endif

   
  even_pre_blanklines   = odd_pre_blanklines  = pre_blanklines  / 2 ;
  even_non_blanklines   = odd_non_blanklines  = non_blanklines  / 2 ;
  even_post_blanklines  = odd_post_blanklines = post_blanklines / 2 ;
  
  start_even= 0;
  start_odd = 1;
  
  if ( (pre_blanklines %2 == 0) &&
       !(non_blanklines % 2 == 0) && 
       !(post_blanklines % 2 == 0)) { 
    even_non_blanklines++ ;
    odd_post_blanklines++;
    start_even= 0;
    start_odd = 1;
  }
  if ( !(pre_blanklines %2 == 0) &&
       !(non_blanklines % 2 == 0) && 
       (post_blanklines % 2 == 0)){ 
    even_pre_blanklines++;
    odd_non_blanklines++;
    start_even= 1;
    start_odd = 0;
  }
  if ( !(pre_blanklines %2 == 0) &&
       (non_blanklines % 2 == 0) && 
       !(post_blanklines % 2 == 0)){ 
    even_pre_blanklines++;
    odd_post_blanklines++;
    start_even= 1;
    start_odd = 0;
  }
  
  width = right - left ;
  
  evenlines = even_pre_blanklines + even_non_blanklines + even_post_blanklines;
  oddlines = odd_pre_blanklines + odd_non_blanklines + odd_post_blanklines;
  
  assert ( oddlines == evenlines );
  
  for (i = (width + 1) * 6  - 1 ; i >= 0; i--)  {
    int color, x, odd, y ;
    j = (width + 1) * 6  - 1 - i ;
    if (i < Left_size ){
      if (!GET_USABLE (Left_r2l, i)){
	memset (place, 0x00, evenlines);
	place += evenlines;
	continue;
      }
      odd = GET_ODD (Left_r2l, i  );
      color = GET_COLOR (Left_r2l, i);
	  x = GET_X (Left_r2l, i );
    } else if (j < Right_size ) {
      if (!GET_USABLE (Right_r2l,j)) {
	memset (place, 0x00, evenlines);
	place += evenlines;
	continue;
      }
      color = GET_COLOR (Right_r2l, j);
      odd = GET_ODD (Right_r2l, j);
      x = width  - 3 - GET_X (Right_r2l, j);
    } else {
      color = (i / 2 - 15) % 3;
      odd = (i + 1) % 2;
      x = (i - 24) / 6 - 1 + odd + (color == 0 ? 0 :/* cyan */
				    color == 1 ? 4 :/* magenta */
				    8);	         /* yellow */
    }
    if (x + left < page_limit->left || x + left >= page_limit->right){
      /* never print data that is outside the limits */
      memset (place, 0x00, evenlines);
      place += evenlines;
    } else {
      int new_pre_blanklines , new_non_blanklines , new_post_blanklines;
      int new_start;
      if (odd) {
	/* odd lines */
	new_pre_blanklines  = odd_pre_blanklines  ;
	new_non_blanklines  = odd_non_blanklines  ;
	new_post_blanklines = odd_post_blanklines ;
	new_start = start_odd;
      } else {
	/* even lines */
	new_pre_blanklines  = even_pre_blanklines  ;
	new_non_blanklines  = even_non_blanklines  ;
	new_post_blanklines = even_post_blanklines ;
	new_start = start_even;
      }
      if ( new_pre_blanklines > 0 ) {
	/* first pre_blanklines lines are blank */
	memset(place, 0x00, new_pre_blanklines ) ;
	place += new_pre_blanklines ;
      }
      for (y = 0; y < new_non_blanklines ; y++) {
	int index = ( (y * 2 + new_start) * data_width * 3 + 
		  (x + left ) * 3 + color ) ;
#ifdef DEBUG 	
	if (!( index < data_size  && index >= 0) ){
	  DPRINTF("index=%d, datasize= %d =  %d x %d\n",
		  index, data_size, data_width, swath_limits->non_blanklines);
	}
	assert ( index  < data_size   && index >= 0 );
#endif
	*place++ = data[index];
      }
      if ( new_post_blanklines > 0 ){
	      /* last post_blanklines lines are blank */
	memset(place, 0x00, new_post_blanklines ) ;
	place += new_post_blanklines  ;
	    }
    }
  }
  return place ;
}	    

