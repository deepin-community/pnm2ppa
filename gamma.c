/***************************************************************************
    gamma.c  -  code for producing color calibration output 
                             -------------------
    begin                : Wed Oct 11  2000
    copyright            : (C) 2000 by pnm2ppa project
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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "global.h"
#include "defaults.h"

#include "image.h"
#include "gamma.h"


int
Make_GammaCurve ( double Gamma_R, double Gamma_G, double Gamma_B)
{
  /*color correction curve derived from  Gamma_R, Gamma_G, Gamma_B .
   Gamma_R = Gamma_G = Gamma_B = 1 gives no correction*/
  int i;
  for (i = 0; i < 256; i++)
    {
      gEnh_curve_r[i] = (int) (pow ((double) i / 256, Gamma_R) * 256);
      gEnh_curve_g[i] = (int) (pow ((double) i / 256, Gamma_G) * 256);
      gEnh_curve_b[i] = (int) (pow ((double) i / 256, Gamma_B) * 256);
    }
  return 0;
}


double
GammaValue( int i )
{
  double  Gamma;
  Gamma = (double) (1.0 - 0.033 * i);
  return Gamma;
}



int
ProduceGamma (image_t *image, unsigned char *line )
{
    int i, j, q;
    double Gamma_R, Gamma_B, Gamma_G;

    /* produces a single line of the gamma calibration output */


    /* increase j to the value  for this line */
    for (j = 0; j * (image->height / 20) < image->bufferCurLine - 150; j++);
    Gamma_R = Gamma_B = Gamma_G = GammaValue (j);
    Make_GammaCurve ( Gamma_R, Gamma_G, Gamma_B);
    
    memset (line, 0xff, image->width * 3);
    
    if (((image->bufferCurLine - 150) % (image->height / 20) > 64))
      {
	for (i = 150; i < image->width / 4; i++)
	  {
	    q = (unsigned char) (255.0 * (i - 150) / (image->width / 4 - 150));
	    /* R */
	    line[3 * i] = q;
	    line[3 * i + 1] = 0;
	    line[3 * i + 2] = 0;
	    
	    /* G */
	    line[3 * (i + image->width / 4)] = 0;		
	    line[3 * (i + image->width / 4) + 1] = q;
	    line[3 * (i + image->width / 4) + 2] = 0;
	    
	    /* B */
	    line[3 * (i + 2 * image->width / 4)] = 0;
	    line[3 * (i + 2 * image->width / 4) + 1] = 0;
	    line[3 * (i + 2 * image->width / 4) + 2] = q;
	      }
      }
    else
      for (i = 150; i < image->width / 4; i += (image->width / 40 - 15))
	{
	  line[3 * i] = 
	    line[3 * i + 1] = 
	    line[3 * i + 2] = 0;
	  
	  line[3 * (i + image->width / 4) + 1] =
	    line[3 * (i + image->width / 4)] =
	    line[3 * (i + image->width / 4) + 2] = 0;

	  line[3 * (i + 2 * image->width / 4) + 2] =
	    line[3 * (i + 2 * image->width / 4)] =
	    line[3 * (i + 2 * image->width / 4) + 1] = 0;
	  
	  line[3 * (i + 1)] = 
	    line[3 * (i + 1) + 1] =
	    line[3 * (i + 1) + 2] = 0;
	  
	  line[3 * (i + 1 + image->width / 4) + 1] =
	    line[3 * (i + 1 + image->width / 4)] =
	    line[3 * (i + 1 + image->width / 4) + 2] = 0;
	  
	  line[3 * (i + 1 + 2 * image->width / 4) + 2] =
	    line[3 * (i + 1 + 2 * image->width / 4)] =
	    line[3 * (i + 1 + 2 * image->width / 4) + 1] = 0;
	  
	}
    
    
    if ((j == 1) || (j == 2 && (image->height / 20) > 64))
      {
	/* this line should be written to the ppm output file */
	return 1;
	  }
    return 0;
}







