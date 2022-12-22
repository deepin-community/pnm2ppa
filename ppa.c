/***************************************************************************
    ppa.c  -  functions to handle PPA, SCP, and VLINK protocols
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define __PPA_C__

#include "ppa_syslog.h"
#include "global.h"
#include "ppa.h"
#include "debug.h"
#include "lang.h"


/*
  VLink packet structure:

    Bytes  Description
    --------------------------------------------
      1    Packet-start marker (always '$')
      1    Channel (0: image data, 1: commands/responses(*), 2: autostatus(*), 128: pacing(*))
      2    Data length (N)
      N    [remaining data (e.g., SCP or SCP2 packet when Channel=1)]

  (*): responses, autostatus, and pacing are communicated from the printer to
       the computer, and may be safely ignored.
*/
void
vlink_put (FILE * fptr, int channel, int length, void *data)
{
    fputc ('$', fptr);
    fputc (channel, fptr);
    fputc (length >> 8, fptr);
    fputc (length & 0xFF, fptr);
    fwrite (data, length, 1, fptr);
}

/*
  SCP packet structure:

    Bytes  Description
    --------------------------------------------
      2    Command specifier
      2    Command reference number
      1    Priority
      1    padding? (always zero)
      2    Data length (N)
      N    [remaining data]

    ComSpec  ComRef     Command
    -------------------------------
       35       1     Initialize1
      101       2     Initialize2
       21       1     Initialize3
       19       1     Handle Media
       18       1     Print Sweep
*/
void
scp_put (FILE * fptr, int comspec, int comref, int priority,
	 int length, void *data)
{
    /* encapsulate the vlink_put call in here, to avoid a memcpy */
    fputc ('$', fptr);
    fputc (1, fptr);
    fputc ((length + 8) >> 8, fptr);
    fputc ((length + 8) & 0xFF, fptr);

    fputc (comspec >> 8, fptr);
    fputc (comspec & 0xFF, fptr);
    fputc (comref >> 8, fptr);
    fputc (comref & 0xFF, fptr);
    fputc (priority, fptr);
    fputc (0, fptr);
    fputc (length >> 8, fptr);
    fputc (length & 0xFF, fptr);

    fwrite (data, length, 1, fptr);
}


/*
  SCP2 packet structure:

    Bytes  Description
    --------------------------------------------
      2    Command specifier
      2    Packet length (N)
      1    Priority
      1    padding? (always zero)
      2    Command reference number
      4    Channel 0 buffer increment
      4    version number? (always 00 02 00 00)
     N-16  [remaining data]

    ComSpec  ComRef     Command
    -------------------------------
     0x186      1     Initialize1
     0x18f      2     Initialize2
     0x183      1     Initialize3
     0x181      1     Handle Media
     0x180      1     Print Sweep
*/
void
scp2_put (FILE * fptr, unsigned short comspec,
	  unsigned short pkt_len_s16,
	  unsigned char priority, unsigned short comref,
	  unsigned data_len, void *data)
{
    /* encapsulate the vlink_put call in here, to avoid a memcpy */
    fputc ('$', fptr);
    fputc (1, fptr);
    fputc ((pkt_len_s16 + 16) >> 8, fptr);
    fputc ((pkt_len_s16 + 16), fptr);

    fputc (comspec >> 8, fptr);
    fputc (comspec, fptr);
    fputc ((pkt_len_s16 + 16) >> 8, fptr);
    fputc ((pkt_len_s16 + 16), fptr);
    fputc (priority, fptr);
    fputc (0, fptr);
    fputc (comref >> 8, fptr);
    fputc (comref, fptr);
    fputc (data_len >> 24, fptr);
    fputc (data_len >> 16, fptr);
    fputc (data_len >> 8, fptr);
    fputc (data_len, fptr);
    fputc (0, fptr);
    fputc (2, fptr);
    fputc (0, fptr);
    fputc (0, fptr);

    fwrite (data, pkt_len_s16, 1, fptr);
}


/*
  SCP3 packet structure:

    Bytes  Description
    --------------------------------------------
      2    Command specifier
      2    Packet length (N)
      1    Priority
      1    padding? (always zero)
      2    Command reference number
      4    Channel 0 buffer increment
      4    version number? (always 01 04 00 00)
     N-16  [remaining data]

    ComSpec  ComRef     Command
    -------------------------------
     0x186      1     Initialize1
     0x18C     16     Initialize Printer name
     0x1A1      1     Initialize4?
     0x18f      2     Initialize2
     0x183      1     Initialize3
     0x181      1     Handle Media
     0x180      1     Print Sweep
*/
void
scp3_put (FILE * fptr, unsigned short comspec,
	  unsigned short pkt_len_s16,
	  unsigned char priority, unsigned short comref,
	  unsigned data_len, void *data)
{
    /* encapsulate the vlink_put call in here, to avoid a memcpy */
    fputc ('$', fptr);
    fputc (1, fptr);
    fputc ((pkt_len_s16 + 16) >> 8, fptr);
    fputc ((pkt_len_s16 + 16), fptr);

    fputc (comspec >> 8, fptr);
    fputc (comspec, fptr);
    fputc ((pkt_len_s16 + 16) >> 8, fptr);
    fputc ((pkt_len_s16 + 16), fptr);
    fputc (priority, fptr);
    fputc (0, fptr);
    fputc (comref >> 8, fptr);
    fputc (comref, fptr);
    fputc (data_len >> 24, fptr);
    fputc (data_len >> 16, fptr);
    fputc (data_len >> 8, fptr);
    fputc (data_len, fptr);
    fputc (1, fptr);
    fputc (4, fptr);
    fputc (0, fptr);
    fputc (0, fptr);

    fwrite (data, pkt_len_s16, 1, fptr);
}


void
ppa_init_job (ppaPrinter_t * printer)
{
    unsigned char init1[8] =

	{ 0x00, 0x00, 0x01, 0xf4, 0x01, 0x00, 0x00, 0x00 };
    unsigned char init2[4] = { 0xde, 0xad, 0xbe, 0xef };
    unsigned char init3[8] =

	{ 0xde, 0xad, 0xbe, 0xef, 0x02, 0x00, 0x00, 0x00 };
    unsigned char init4[60] = "!!TAZ            \x81*HP DeskJet 1000C Prin (Copy 2)*FILE!!\x00\x00\x00";	/* plus 0 terminator */
    unsigned char init5[4] = { 0x01, 0x01, 0x00, 0x00 };

    switch (printer->version)
    {
    case HP820:
	scp_put (printer->fptr, 35, 1, 7, sizeof (init1), init1);
	vlink_put (printer->fptr, 0, sizeof (init2), init2);
	scp_put (printer->fptr, 101, 2, 7, sizeof (init3), init3);
	break;
    case HP7X0:
	scp2_put (printer->fptr, 0x0186, sizeof (init1), 7, 1, 0, init1);
	vlink_put (printer->fptr, 0, sizeof (init2), init2);
	scp2_put (printer->fptr, 0x018f, sizeof (init3), 7, 2, 4, init3);
	break;
    case HP1000:
	scp3_put (printer->fptr, 0x0186, sizeof (init1), 7, 16, 0, init1);
	scp3_put (printer->fptr, 0x018C, sizeof (init4), 7, 1, 0, init4);
	scp3_put (printer->fptr, 0x01A1, sizeof (init5), 7, 1, 0, init5);
	vlink_put (printer->fptr, 0, sizeof (init2), init2);
	scp3_put (printer->fptr, 0x018f, sizeof (init3), 7, 2, 4, init3);
	break;
    default:
      snprintf(syslog_message,message_size,"ppa_init_job(): %s", 
		gMessages[E_PPA_UNKNOWN]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message);

    }
}

void
ppa_end_print (ppaPrinter_t * printer)
{
    unsigned char pageA[4] = { 0x05, 0x01, 0x03, 0x84 };

    if (printer->version == HP1000)
	scp3_put (printer->fptr, 0x0181, sizeof (pageA), 7, 2, 0, pageA);
}

void
ppa_init_page (ppaPrinter_t * printer)
{
    unsigned char pageA[16] =
	{ 0x28, 0x2d, 0x00, 0x41, 0x29, 0x2e, 0x00, 0x42,
	0x29, 0x2e, 0x00, 0x42, 0x29, 0x2e, 0x00, 0x42
    };
    unsigned char pageB[16] =
	{ 0x28, 0x2d, 0x00, 0x41, 0x2d, 0x32, 0x00, 0x46,
	0x2d, 0x32, 0x00, 0x46, 0x2d, 0x32, 0x00, 0x46
    };
    
    switch (printer->version)
      {
      case HP820:
	scp_put (printer->fptr, 21, 1, 5, sizeof (pageA), pageA);
	break;
      case HP7X0:
	scp2_put (printer->fptr, 0x0183, sizeof (pageB), 5, 1, 0, pageB);
	break;
      case HP1000:
	scp3_put (printer->fptr, 0x0183, sizeof (pageA), 5, 1, 0, pageA);
	break;
      default:
      snprintf(syslog_message,message_size,"ppa_init_page(): %s",  
		gMessages[E_PPA_UNKNOWN]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message);

      }
}

void
ppa_load_page (ppaPrinter_t * printer)
{
    unsigned char loadA[4] = { 0x01, 0x01, 0x09, 0x60 };
    unsigned char loadB[4] = { 0x01, 0x01, 0x12, 0xc0 };
    unsigned char loadC[4] = { 0x01, 0x01, 0x07, 0x08 };

    switch (printer->version)
    {
    case HP820:
	scp_put (printer->fptr, 19, 1, 7, sizeof (loadA), loadA);
	break;
    case HP7X0:
	scp2_put (printer->fptr, 0x0181, sizeof (loadB), 7, 1, 0, loadB);
	break;
    case HP1000:
	scp3_put (printer->fptr, 0x0181, sizeof (loadC), 7, 1, 0, loadC);
	break;
    default:
      snprintf(syslog_message,message_size, "ppa_load_page(): %s",   
		gMessages[E_PPA_UNKNOWN]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message);

    }
}

void
ppa_eject_page (ppaPrinter_t * printer)
{
    unsigned char loadA[4] = { 0x02, 0x01, 0x09, 0x60 };
    unsigned char loadB[4] = { 0x02, 0x01, 0x12, 0xc0 };
    unsigned char loadC[4] = { 0x02, 0x01, 0x07, 0x08 };

    switch (printer->version)
    {
    case HP820:
	scp_put (printer->fptr, 19, 1, 7, sizeof (loadA), loadA);
	break;
    case HP7X0:
	scp2_put (printer->fptr, 0x0181, sizeof (loadB), 7, 1, 0, loadB);
	break;
    case HP1000:
	scp3_put (printer->fptr, 0x0181, sizeof (loadC), 7, 1, 0, loadC);
	break;
    default:
      snprintf(syslog_message,message_size, "ppa_eject_page(): %s",   
		gMessages[E_PPA_UNKNOWN]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message);
     }
}

int
compress (unsigned char *in, int num_lines_d2, int final_len,
	  unsigned char *iout)
{
    unsigned char *out = iout;
    int I, len = num_lines_d2;
    unsigned compressed_len = 0;


    /* abandon compression if datasize expands!  */

    for (I = 0; I < final_len; I += num_lines_d2, in += num_lines_d2)
    {
	int i = 0;

	while (i < len)
	{
	    /* Find the size of duplicate values */
	    int dup_len = 0;

	    while ((i + dup_len < len) && (in[i + dup_len] == in[i]))
	    {
		dup_len++;
	    }
	    /* See if we have enough zeros to be worth compressing. */
	    /* I figure one is enough. */
	    if ((dup_len >= 1) && (in[i] == 0))
	    {
		/* Output run of zeros. */
		while (dup_len >= 128)
		{
		    /* Max is 128 */
		    *out++ = 0x00;
		    compressed_len++;
		    if (compressed_len > final_len)
		      return 0;
		    i += 128;
		    dup_len -= 128;
		}
		if (dup_len >= 1)
		{
		    *out++ = dup_len;
		    compressed_len++;
		    if (compressed_len > final_len)
		      return 0;

		    i += dup_len;
		}
		/* See if we have enough non-zeros to be worth compressing. */
		/* Here two should be enough. */
	    }
	    else if (dup_len >= 2)
	    {
		/* Output run of duplicates. */
		while (dup_len >= 64)
		{
		    /* Max is 64 */
		    *out++ = (char) 0x80;
		    *out++ = (char) in[i];
		    compressed_len +=2;
		    if (compressed_len > final_len)
		      return 0;

		    i += 64;
		    dup_len -= 64;
		}
		if (dup_len >= 2)
		{
		    *out++ = dup_len + 0x80;
		    *out++ = in[i];
		    compressed_len +=2;
		    if (compressed_len > final_len)
		      return 0;

		    i += dup_len;
		}
	    }
	    else
	    {
		/* Look for two zeros, or three duplicates to end literal run. */
		/* Note this is one more than the number to start a run. */
		int lit_len = -1;
		int add_more = 1;

		while (add_more)
		{
		    lit_len++;
		    if (i + lit_len == len)
			add_more = 0;
		    /* Always add more if we are near the very end. */
		    if (i + lit_len < len - 3)
		    {
			char a = in[i + lit_len + 0];
			char b = in[i + lit_len + 1];
			char c = in[i + lit_len + 2];

			/* See if there are enough zeros */
			if ((a == b) && (b == 0))
			    add_more = 0;
			/* See if there are enough duplicates */
			if ((a == b) && (b == c))
			    add_more = 0;
		    }
		}
		/* Output run of literals. */
		while (lit_len >= 64)
		{
		    /* Max is 64 */
		    int j;

		    *out++ = (char) 0xc0;
		    compressed_len ++;
		    for (j = i; j < i + 64; j++)
		    {
			*out++ = in[j];
		    }
		    compressed_len += 64;
		    if (compressed_len > final_len)
		      return 0;
		    i += 64;
		    lit_len -= 64;
		}
		if (lit_len)
		{
		    int j;

		    *out++ = lit_len + 0xc0;
		    compressed_len ++;
		    for (j = i; j < i + lit_len; j++)
		    {
			*out++ = in[j];
		    }
		    compressed_len += lit_len;
		    if (compressed_len > final_len)
		      return 0;
		    i += lit_len;
		}
	    }
	}
    }
    if (compressed_len > final_len)
      return 0;

    return out - iout;
}

static inline void
packShort (int x, unsigned char *y)
{
    y[0] = x >> 8;
    y[1] = x;
}

static inline void
packLong (unsigned long x, unsigned char *y)
{
    packShort(x >> 16, y);
    packShort(x, y + 2);
#if 0
    unsigned short t;

    t = (unsigned short) ((x & 0xffff0000) >> 16);
    packShort (t, y);
    packShort ((unsigned short) x & 0xffff, y + 2);
#endif
}

/*#define do_compress_data (1) */

void
ppa_print_sweep (ppaPrinter_t * printer, ppaSweepData_t * data)
{
  unsigned char *pc, *tpc;
  unsigned i, datasize = data->data_size;
  unsigned char sweep_packet[144];
  int sweep_packet_size;
  unsigned short HP7X0constants[] = { 0x8ca0, 0x4650, 0x12c0 };
  unsigned short HP820constants[] = { 0x4650, 0x1c20, 0x0960 };
  unsigned short HP1000constants[] = { 0x4650, 0x2328, 0x0708 };
  unsigned short *constants;
  int nozzle_data_size;
  int MF;			/* Multiplicative Factor -- quick hack */
  
  unsigned char do_compress_data;
  
  DPRINTF("ppa_print_sweep(): Dir: %d Color: %d Size: %d Vpos: %d Hpos: %d,%d\n", 
	  data->direction, data->in_color, data->data_size, data->vertical_pos, 
	  data->left_margin, data->right_margin );
  assert( ((data->direction == left_to_right) || (data->direction == right_to_left)) );
  
  if (data->in_color && !gCompress )
    do_compress_data = 0;
  else
    do_compress_data = 1;
  
  
  
  pc = data->image_data;
  
  DPRINTF(" ppa_print_sweep(): datasize= %d\n", datasize);
  
  if (do_compress_data) {
    unsigned compressed_datasize;
    if (!(pc = malloc ((datasize / 64 + 1) * 65)))
      {
	snprintf(syslog_message,message_size, "ppa_print_sweep(): %s",   
		 gMessages[E_PPA_BADMALLOC]);
	wrap_syslog (LOG_CRIT,"%s",syslog_message);
	  exit (-1);
      }
    compressed_datasize =
      compress (data->image_data,
		data->nozzle_data->pins_used_d2, datasize, pc);
    if (!compressed_datasize) {
      /* compression would increase datsize ! */
      free (pc);
	  do_compress_data = 0;
	  pc = data->image_data;
    } else {
      datasize = compressed_datasize;
      DPRINTF(" ppa_print_sweep():compressed datasize= %d\n", datasize);
    }
    
  }
  
  /* send image data 16k at a time */
  for (i = 0, tpc = pc; i < datasize; tpc += 16384, i += 16384)
    {
      vlink_put (printer->fptr, 0, ((datasize - i) > 16384) ? 16384 : (datasize - i), tpc);
    }
  
    
  /* memory leak fix courtesy of John McKown */
  if (do_compress_data && pc){
    free (pc);
    pc = NULL;
  }
  
  /* construct sweep packet */
  switch (printer->version) {
  case HP820:
    constants = HP820constants;
    MF = 1;
    break;
  case HP7X0:
    constants = HP7X0constants;
	    MF = 2;
	    break;
  case HP1000:
    constants = HP1000constants;
    MF = 1;
    break;
  default:
    snprintf(syslog_message,message_size, "ppa_print_sweep(): %s",
	     gMessages[E_PPA_UNKNOWN]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message);
	      return;
  }
  
  sweep_packet[0] = 0;
  sweep_packet[1] = do_compress_data;
  sweep_packet[2] = data->direction == right_to_left ? 1 : 2;
  sweep_packet[3] = data->in_color ? 14 : 1;
  
  packLong (datasize, sweep_packet + 4);
  
  memset (sweep_packet + 8, 0, 8);
  
  if (data->in_color)
    packLong (MF * (data->vertical_pos +
		    /*96+ */ printer->y_offset) /*+ 1*/,
	      sweep_packet + 16);
  else
    packLong (MF *
	      (data->vertical_pos +
	       printer->y_offset), sweep_packet + 16);
  packShort (constants[0], sweep_packet + 20);
  packShort (MF *
	     (data->left_margin + printer->x_offset) /*+ 1*/,
	     sweep_packet + 22);
  packShort (MF *
	     (data->right_margin + printer->x_offset) /*+ 1*/,
	     sweep_packet + 24);
  if (data->in_color)
    packShort (0x2ee0, sweep_packet + 26);
  else
    packShort (constants[1], sweep_packet + 26);	/* 2ee0 in color on 820 */
  packShort (constants[2], sweep_packet + 28);
  packShort (0x100, sweep_packet + 30);
  
  
  if (data->next) {
    sweep_packet[32] =
      data->next->direction == right_to_left ? 1 : 2;
    sweep_packet[33] = data->next->in_color ? 14 : 1;
    packLong (MF *
	      (data->next->vertical_pos +
	       printer->y_offset), sweep_packet + 34);
    packShort (MF *
	       (data->next->left_margin +
		printer->x_offset), sweep_packet + 38);
    packShort (MF *
	       (data->next->right_margin +
		printer->x_offset), sweep_packet + 40);
    if (data->in_color)
      packShort (0x2ee0, sweep_packet + 42);
    else
      packShort (constants[1], sweep_packet + 42);	/* 2ee0 in color on 820 */
		packShort (constants[2], sweep_packet + 44);
  }
  else
    memset (sweep_packet + 32, 0, 14);
  sweep_packet[46] = 8;
  
  nozzle_data_size = data->nozzle_data_size;
  if (nozzle_data_size > 6) {
    snprintf(syslog_message,message_size, "ppa_print_sweep(): %s %d\n",
	     gMessages[W_PPA_NOZZLE], nozzle_data_size); 
    wrap_syslog (LOG_WARNING,"%s",syslog_message);
    
    nozzle_data_size = 6;
  }
  sweep_packet[47] = nozzle_data_size;
  
  for (pc = sweep_packet + 48, i = 0;
       i < nozzle_data_size; i++, pc += 16) {
    packShort (data->nozzle_data[i].DPI, pc);
    packShort (data->nozzle_data[i].pins_used_d2, pc + 2);
    packShort (data->nozzle_data[i].unused_pins_p1, pc + 4);
    packShort (data->nozzle_data[i].first_pin, pc + 6);
    packShort (data->nozzle_data[i].pins_used_d2, pc + 8);
    packShort (MF * (data->nozzle_data[i].left_margin + printer->x_offset),
	       pc + 10);
    packShort (MF * (data->nozzle_data[i].right_margin + printer->x_offset),
	       pc + 12);
    pc[14] = data->nozzle_data[i].nozzle_delay;
    pc[15] = 0;
  }
  
  sweep_packet_size = data->in_color ? 144 : 80;
  
  
  /* send sweep packet */
  switch (printer->version) {
  case HP820:
    scp_put (printer->fptr, 18, 1, 7, sweep_packet_size, sweep_packet);
    break;
  case HP7X0:
    scp2_put (printer->fptr, 0x0180, sweep_packet_size,
		      7, 1, datasize, sweep_packet);
    break;
  case HP1000:
    scp3_put (printer->fptr, 0x0180, sweep_packet_size,
	      7, 1, datasize, sweep_packet);
    break;
  default:
    snprintf(syslog_message,message_size,"ppa_print_sweep(): %s",
	     gMessages[E_PPA_UNKNOWN]);
    wrap_syslog (LOG_CRIT,"%s",syslog_message); 
    
    return;
  }
}



void
ppa_print_sweeps (ppaPrinter_t * printer, ppaSweepData_t * data)
{
    ppaSweepData_t *current_sweep;

    for (current_sweep = data; current_sweep;
	 current_sweep = current_sweep->next)
	ppa_print_sweep (printer, current_sweep);
}
