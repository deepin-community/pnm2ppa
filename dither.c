/***************************************************************************
    dither.c  -  floyd-steinberg & halftone dither routines 
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
#include <assert.h>

#define __DITHER_C__

#include "global.h"
#include "debug.h"

#include "image.h"
#include "fscoeff.h"
#include "palette.h"
#include "hash_ink.h"

int
ink_lookup(int R, int G, int B, int MaxPass, 
	   float *d, int *bx, int *by, int *bz, int *br, int *bg, int *bb)
{ /* Try to find correct color value in hashed structure instead of 
   * exhaustive search in (MaxPass+1)^3 ink possibilities */

  struct hash_ink **arr = 0, *cur;
  int r, g, b, h;
  float d1;
  int shift = 0; /* Keep the compiler quiet! */
  
  /* *d = -1; */
  if (MaxPass == 4) /* standard mode */
    {
      arr = arr_max4;
      shift = shift4;
    }
  else if (MaxPass == 1) /* --eco mode */
    {
      arr = arr_max1;
      shift = shift1;
    }
  /* Add here other supported MaxPass arrays */

  if (arr == 0) /* not supported MaxPass value by int_lookup() */
    return 0;

  r = R >> shift;
  g = G >> shift;
  b = B >> shift;
  
  h = (r << (16 - shift * 2)) + (g << (8-shift)) + (b); 
  /* h e [0, 4096> for shift = 4 */
  
  cur = arr[h];

  while( cur != 0 )
  /* the code added below is an edited copy of the code found below
     in FS_Color_Dither */
  {
    r =	(int) R - (int) cur->data[0];
    g = (int) G - (int) cur->data[1];
    b = (int) B - (int) cur->data[2];
			      
    d1 = /* 0.5 * */ r * r + g * g + b * b;


    if (d1 <= *d)
      {
	*d = d1;
	*bx = gMaxPass + 1 - cur->x;
	*by = gMaxPass + 1 - cur->y;
	*bz = gMaxPass + 1 - cur->z;
	*br = r;
	*bg = g;
	*bb = b;
      }

    cur = cur->next;
  }
  
  return MaxPass;
}

void
FS_Color_Dither (image_t *image, unsigned char *line)
{
  short dd;
  int i, i_max ;
  int x, bx, y, by, z, bz;
  float d, d1;
  int R, G, B, dr, dg, db;
  int br, bg, bb, r, g, b, oldr, oldb, oldg;
  static short *err_coeff_e = 0;
  static short *err_coeff_se = 0;
  static short *err_coeff_s = 0;
  static short *err_coeff_sw = 0;
  int MaxPass = 0;
  int *error;
  
  if (image->bufferCurLine % 2 == 1)
    {
      error = gErrVec_bw;
    }
  else
    {
      error = gErrVec;
      MaxPass = gMaxPass;
    }
  
  r = g = b = dr = dg = db = 0;
  br = r;
  bg = g;
  bb = b;
  
  if (err_coeff_e == NULL)
    {
      err_coeff_e = fs_err_coeff_e + 255;
      err_coeff_se = fs_err_coeff_se + 255;
      err_coeff_s = fs_err_coeff_s + 255;
      err_coeff_sw = fs_err_coeff_sw + 255;
      
      dd = 25;
      
      for (i = -255; i < 256; i++)
	{
	  err_coeff_e[i] = i * 8 / dd;	
	  err_coeff_se[i] = i * 5 / dd;	
	  err_coeff_s[i] = i * 8 / dd;	
	  err_coeff_sw[i] = i * 4 / dd;
	}
    }
  
  /* Only even lines are treated for color ;
     pixels are treated in pairs, and
     reduced to 300dpi resolution */
   
      i_max = image->width * 3 ;

  if (image->bufferCurLine % 2 == 0)
    {
      /*treatment of even lines */
    for (i = 0; i < i_max ; i += 6)
      {
	
	oldr = br;
	oldb = bb;
	oldg = bg;
	
	br = bg = bb = 0;

	      /* avoid the (0,0,0) entry for black if 
	       * no match is found (initial d too small)
	       * use (1,1,1) instead (gMaxPass drops of each color)
	       */
        bx = by = bz = 1      ;

	/* if either of a pair of successive pixels is black,
	   make  both black (produces 300dpi black resolution) */
	if ( (i+3) != i_max)
	  {
	   if ( !line[i + 3] && !line[i + 4] && !line[i + 5] )
		 line[i]=  line[i + 1] = line[i + 2]  = 0 ;
	   }

	if ( !line[i] && !line[i + 1] && !line[i + 2] )
	  {
	    /* pixel is black */
	    error[i + 0] = 0;
	    error[i + 1] = 0;
	    error[i + 2] = 0;
	    dr = dg = db = 0;
	    /* make next pixel black, too */
	    if ( (i + 3) != i_max )
	      {
		 line[i + 3] =  line[i + 4] = line[i + 5]  = 0 ;
	       }
	  }
	else
	  if (((line[i] < 255) || (line[i + 1] < 255)
	       || (line[i + 2] < 255)))
	    {
	      /* pixel is colored */
	      
	      /* 
		 find the "requested color by considering previous 
		 errors
	      */
	      
	      
	      R =
		gEnh_curve_r[(int) (line[i] + line[i + 3]) / 2] +
		error[i] + dr;
	      R = (R < 0 ? 0 : R);
	      R = (R > 255 ? 255 : R);
	      G =
		gEnh_curve_g[(int) (line[(i + 1)] + line[i + 4]) / 2] +
		error[i + 1] + dg;
	      G = (G < 0 ? 0 : G);
	      G = (G > 255 ? 255 : G);
	      B =
		gEnh_curve_b[(int) (line[(i + 2)] + line[i + 5]) / 2] +
		error[i + 2] + db;
	      B = (B < 0 ? 0 : B);
	      B = (B > 255 ? 255 : B);
	      


	      /* 
	       * now convert this RGB value to drops of ink per pixel.
	       * The table is set up as a hashing table .  
	       */


	      if (!gGammaMode)
		{
		  br = R / 2;
		  bg = G / 2;
		  bb = B / 2;
#if 0
		  d = ((2.0 + (float)R / 256.0) * R * R + 4 * G * G + (2 + (255.0 - (float)R) / 256.0) * B * B)*0.7;
#endif
		  if (gMaxPass == 4)
		    {
		      d = (R * R + G * G + B * B) * 1.3;
		    }
		  else
		    {
		      /* does the "1.3"  need adjusting for gMaxPass = 1?
		       * This sets "very dark" pixels to black.
		       */
		      d = (R * R + G * G + B * B) * 1.3;
		    }
		}
	      else
		{
		  /* this is for the special case when gamma.ppm is
		   * being generated.
		   */
		  d = 2147483647; /* just a large initial value */
		}
	      
#ifdef CHECK_HASHING
	      {
		/* code for checking correctness of hash_ink (by ink_lookup)
		   Not sensible in production code! */
		int sbx, sby, sbz, sbr, sbg, sbb;
		float sd;
		int s_result;
		
		sd = d;
		sbx = bx;
		sby = by;
		sbz = bz;
		sbr = br;
		sbg = bg;
		sbb = bb;
		
		s_result = ink_lookup(R, G, B, MaxPass, 
				      &sd, &sbx, &sby, &sbz, &sbr, &sbg, &sbb);
#else
		if (! ink_lookup(R, G, B, MaxPass, 
				 &d, &bx, &by, &bz, &br, &bg, &bb))
		  {
		    /* ink_lookup failed -- so use old code as fallback */
		    static int unsupported_flag = 0;
		    
		    if (unsupported_flag != MaxPass)
		      {
			unsupported_flag = MaxPass;
			DPRINTF("WARNING:Unsupported MaxPass value %d by ink_lookup\n", MaxPass);
		      }
#endif
		    /* this code scans the 625 (gMaxPass=4) or
		     * 8 (gMaxPass=1) possible combinations of
		     * (z,y,x) drops of (Cyan,Magenta,Yellow) ink
		     * to best match the RGB value for the pixel, 
		     * using the tabulated values in hp_pal
		     * in pallette.h
		     */

		    for (z = 0; z < MaxPass + 1; z++)
		      for (x = 0; x < MaxPass + 1; x++)
			for (y = 0; y < MaxPass + 1; y++)
			  if (!
			      (((x == 4) && (y > 2) && (z > 2)) ||
			       ((y == 4) && (x > 2) && (z > 2)) ||
			       ((z == 4) && (y > 2) && (x > 2))))
			    
			    {
			      
#if 0
			      rmean = ((int) R + (int) hp_pal[4-x][4-y][4-z][0]) / 2;
#endif
			      
			      r =
				(int) R - (int) hp_pal[4 - x][4 - y][4 -
								    z]
				[0];
			      g =
				(int) G - (int) hp_pal[4 - x][4 - y][4 -
								    z]
				[1];
			      b =
				(int) B - (int) hp_pal[4 - x][4 - y][4 -
								    z]
				[2];
#if 0			      
			      d1 = (2.0 + (float)rmean / 256.0) * r * r + 
				4 * g * g +
				(2 + (255.0 - (float)rmean) / 256.0) * b * b;
#endif
			      d1 = /* 0.5 * */ r * r + g * g + b * b;
			      
			      /* the next test also excludes "too dark"
			       * pixels with a small initial d value.
			       */

			      if (d1 <= d)   
				{
				  d = d1;
				  bx = gMaxPass + 1 - x;
				  by = gMaxPass + 1 - y;
				  bz = gMaxPass + 1 - z;
				  br = r;
				  bg = g;
				  bb = b;
				}
			    }
#ifdef CHECK_HASHING
		    if (s_result != 0)
		      {
			/* if (d != sd)
			   debug( R, G, B, d, gMaxPass + 1 - bx, 
			   gMaxPass + 1 - by, gMaxPass + 1 - bz ); */
			assert(d == sd);
			/*  The assertions below are deleted; there might
			    be equivalent solutions (w.r.t. distance d)
			    with different values for bx, by, bz 
			    assert(sbx == bx);
			    assert(sby == by);
			    assert(sbz == bz);
			    assert(br == sbr);
			    assert(bg == sbg);
			    assert(bb == sbb); */
		      }
		  }
#else
	      }
#endif
	      
	      
	      if ( true  /* || (bx==0 && by==0 && bz==0) */ )
		{
		  /* careful! floats cannot be compared easily */
		  if (!(d == 0.0) && ((i + 8) < image->width * 3))
		    {
		      
		      dr = err_coeff_e[br];
		      dg = err_coeff_e[bg];
		      db = err_coeff_e[bb];
		      
			error[i + 0] = err_coeff_s[br];
			error[i + 1] = err_coeff_s[bg];
			error[i + 2] = err_coeff_s[bb];
			
			if (i > 3)
			  {
			    error[i - 6] += err_coeff_sw[br];
			    error[i - 5] += err_coeff_sw[bg];
			    error[i - 4] += err_coeff_sw[bb];
			  }
			
		    }
		  else
		    error[i + 0] = error[i + 1] =
		      error[i + 2] = dr = dg = db = 0;
		}
	      
	      /* 
	       *Put in line[...] the times each color component 
	       *have to be printed
	       * gMaxPass + 1 - bz = number of drops of Cyan
	       * gMaxPass + 1 - by = number of drops of Magenta
	       * gMaxPass + 1 - bx = number of drops of Yellow
	       */

	      line[i] = (unsigned char) bz;
	      line[i + 1] = (unsigned char) by;
	      line[i + 2] = (unsigned char) bx;
	      if ((i + 3)  != i_max)
		{
		  line[i + 3] = line[i];
		  line[i + 4] = line[i + 1];
		  line[i + 5] = line[i + 2];
		}
	    }
	
	  else
	    {
	      /* pixel is white */
	      error[i + 0] = error[i + 1] = error[i + 2] = dr = dg = db = 0;
	      line[i] = line[i + 1] = line[i + 2] = 255;
	      if ( (i + 3)  != i_max)
		{
		  /*  white out next pixel, */
		    line[i + 3] = line[i + 4] = line[i + 5] = 255;
		}
	    }
	if (i < image->width * 3 - 8)
	  {
	    error[i] += err_coeff_se[oldr];
	    error[i + 1] += err_coeff_se[oldg];
	    error[i + 2] += err_coeff_se[oldb];
	  }
	
      }
    }
  else
    {
      /* treatment of odd lines (black only) */
      for (i = 0; i < i_max ; i += 6)
	{
	      /* if either of two successive pixels are black, 
		 make both black */
	  if ( (i+3) != i_max)
	    {
	      if ( !line[i + 3] && !line[i + 4] && !line[i + 5] )
		line[i] = line[i + 1] = line[i + 2] = 0;
	      else if ( !line[i] && !line[i + 1] && !line[i + 2] )
		line[i + 3] = line[i + 4] = line[i + 5] = 0;
	    }
	  /* white out non-black pixels */
	  
	  if ( line[i] || line[i + 1] || line[i + 2] )
	    {
	      line[i] = line[i + 1] = line [i + 2] = 255 ;
	      if ( (i+3) != i_max )
		line[i + 3] = line[i + 4] = line [i + 5] = 255 ;
	    }
	  
	}
    }
}

void
HT_Black_Dither (image_t *image, unsigned char *line)
{
    int i, j = 0, k, i_max;

    unsigned int halftone_3[5][5] = {
	{120, 180, 450, 270,  30},
	{240, 510, 720, 600, 330},
	{420, 630, 750, 660, 390},
	{360, 570, 690, 540, 210},
	{ 60, 300, 480, 150,  90}
    };

    unsigned int halftone[5][5] = {    
	{ 40,  60, 150,  90,  10},
	{ 80, 170, 240, 200, 110},
	{140, 210, 250, 220, 130},
	{120, 190, 230, 180,  70},
	{ 20, 100, 160,  50,  30}
    };

    k = image->bufferCurLine % 5;

    if (gByteCount == 3)
      {
	i_max = image->width * 3;
	for (i = 0; i < i_max ; i += 3)
	  {
	    /* make  black pixels come in pairs */
	    if ( !(i % 6) && (i + 3) != i_max  )
	      {
		if (!line[i] &&  !line[i + 1] && !line[i + 2] )
		  line[i + 3] = line [i + 4] = line[i + 5] = 0;
		else
		  if (!line[i + 3] &&  !line[i + 4] && !line[i + 5] )
		    line[i] = line [i + 1] = line[i + 2] = 0;
	      }
	    
	    
	    if (line[i] + line[i + 1] + line[i + 2]  < halftone_3[j][k])
		line[i] = line[i + 1] = line[i + 2] = 0;
	    else
		line[i] = line[i + 1] = line[i + 2] = 255;

	    j++;
	    j %= 4;
	  }
      }
    else    /* gByteCount = 1 */
      {
	i_max = image->width;
	for (i = 0; i < i_max ; i++)
	  {
	    /* make  black pixels come in pairs */
	    if ( !(i % 2) && (i + 1) != i_max  )
	      {
		if (!line[i])
		  line[i + 1] = 0;
		else
		  if (!line[i + 1] )
		    line[i] = 0;
	      }
	    
	    if (line[i]  < halftone[j][k])
	      line[i] = 0;
	    else
	      line[i] = 255;
	    
	    j++;
	    j %= 4;
	    
	  }
      }	
}

void
HT_Color_Dither (image_t *image, unsigned char *line)
{
    int i = 0, i_max;

    unsigned char halftoner[5][5] = {
	{40, 60, 150, 90, 10},
	{80, 170, 240, 200, 110},
	{140, 210, 250, 220, 130},
	{120, 190, 230, 180, 70},
	{20, 100, 160, 50, 30}
    };
    unsigned char halftoneg[5][5] = {
	{40, 80, 140, 120, 20},
	{60, 170, 210, 190, 100},
	{150, 240, 250, 230, 160},
	{90, 200, 220, 180, 50},
	{10, 110, 130, 70, 30}
    };
    unsigned char halftoneb[5][5] = {
	{100, 230, 220, 110, 40},
	{60, 160, 180, 130, 80},
	{150, 50, 70, 140, 170},
	{240, 90, 30, 120, 210},
	{200, 10, 20, 190, 250}
    };

  /* Only even lines are evaluated ;
     pixels are treated in pairs, and
     reduced to 300dpi resolution */
   

    i_max = image->width * 3 ;
    
    if (image->bufferCurLine % 2 == 0)
      {
	/* treatment of even lines */
      for (i = 0; i < i_max ; i += 6)
	{

	  /* if either of two successive pixels are black, make both black */
	  if ( (i+3) != i_max)
	    {
	      if ( !line[i + 3] && !line[i + 4] && !line[i + 5] )
		line[i] = line[i + 1] = line[i + 2] = 0;
	    }


	  if (line[i] || line[i + 1] || line[i + 2])
	    if (image->bufferCurLine % 2 == 0)
	      {
		if (line[i] < halftoner[i % 5][image->bufferCurLine % 5])
		  line[i] = 1;
		else
		  line[i] = 255;
		if (line[i + 1] < halftoneg[i % 5][image->bufferCurLine % 5])
		  line[i + 1] = 1;
		else
		  line[i + 1] = 255;
		if (line[i + 2] < halftoneb[i % 5][image->bufferCurLine % 5])
		  line[i + 2] = 1;
		else
		  line[i + 2] = 255;
	      }
	      if ( (i+3) != i_max)
		{
		  line[i + 3] = line[i];
		  line[i + 4] = line[i + 1];
		  line[i + 5] = line[i + 2];
		}
	}
      }
      else
	{
	  /* treatment of odd lines (black only) */
	  for (i = 0; i < i_max ; i += 6)
	    {
	      /* if either of two successive pixels are black, 
		 make both black */
	      if ( (i+3) != i_max)
		{
		  if ( !line[i + 3] && !line[i + 4] && !line[i + 5] )
		    line[i] = line[i + 1] = line[i + 2] = 0;
		  else if ( !line[i] && !line[i + 1] && !line[i + 2] )
		    line[i + 3] = line[i + 4] = line[i + 5] = 0;
		}
	      
	      /* white out non-black pixels */
	      if ( line[i] || line[i + 1] || line[i + 2] )
		{
		  line[i] = line[i + 1] = line [i + 2] = 255 ;
		  if ( (i+3) != i_max )
		    line[i + 3] = line[i + 4] = line [i + 5] = 255 ;
		}
	      
	    }
	}
}



