/***************************************************************************
    calibrate_ppa.c  -  program to produce PPM data for pnm2ppa  calibration
                             -------------------
    begin                : Thu Oct 12 2000
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* incorporates color features added by Bruno ALIX */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/param.h>

#define __PNM2PPA_C__
#define __CALIBRATE_PPA_C__
#include "global.h"
#include "image.h"
#include "defaults.h"
#include "pnmtools.h"

int make_ppm_file ( FILE * , image_t *, char *) ;

int make_test_pattern ( FILE *, int, char * );


/* int black_maxlines = 300; */
int black_maxlines = 128;     /*put this back to old value for now (duncan)*/


void
show_usage (char *prog)
{
    printf ("usage: %s [ options ] [ -o <outfile> ]] \n\n",
	    prog);
    printf
           ("  Produces various PixMap (PPM) files used for calibrating pnm2ppa.\n\n");
    printf ("  NOTE: The size of a  full-page PixMap is 100MB!!.\n");  
    printf ("  These PixMaps should be piped directly to pnm2ppa, except in the case of\n");
    printf ("  the  -g option, where the PixMap should be viewed or printed by other means.\n\n");
    printf ("    -g, --gamma     produce  gamma.ppm (10MB, used in color calibration).\n");
    printf ("    -h,   --help    displays this help text.\n");
    printf ("    -o <outfile>    output to file <outfile>, \"-o -\" means stdout (default)\n");
    printf ("    -p <n>          create test pattern number <n>\n");
    printf ("    --align         pattern 0:  align print heads. (default) \n");
    printf ("    --test          pattern 1:  test alignment. \n");
    printf ("    --clean         pattern 2:  clean print head. \n");
    printf ("    --center        pattern 3:  center paper. \n");
    printf ("    -s <paper>      default papersize (a4, legal, letter = us (default))\n");
#if 0
    printf ("    -v <printer>    Printer model:710=712=720=722;820;1000 (default:7xx)\n");
#endif
    printf ("\n");
}

int
main (int argc, char *argv[])
{
  int  opt_index;
  image_t image ;
  char *progname = NULL;
  BOOLEAN use_stdout = true ;
  BOOLEAN gamma = false ;
  FILE *outFile = NULL ;
  int testPattern = 0;
  char *out_filepath = NULL;
  char c;
  static struct option long_opts[] = {
    {"gamma",0,0,'g'},
    {"align",0,0,0},
    {"test",0,0,0},
    {"clean",0,0,0},
    {"center",0,0,0},
    {"help",0,0,'h'},
    {0, 0, 0, 0}
  };

  gWidth = LETTERWIDTH;
  gHeight = LETTERHEIGHT ;
  
  
  /* read through the optional arguments before tring to find filenames */
  
  while ((c = getopt_long (argc, argv, "gho:p:s:v:",
			   long_opts, &opt_index)) != EOF)
    {
      progname = *argv;
      switch (c)
	{
	case 0:
	  /* long options */
	  switch (opt_index)
	    {
	    case 0:
	      /* --gamma */
	      gamma = true;
	      break;
	    case 1:
	      /* --align */
	  gamma = false;
	  testPattern = 0;
	  break;
	    case 2:
	      /* --test */
	  gamma = false;
	  testPattern = 1;
	  break;
	    case 3:
	      /* --clean */
	  gamma = false;
	  testPattern = 2;
	  break;
	    case 4:
	      /* --center */
	  gamma = false;
	  testPattern = 3;
	  break;
	    case 5:
	      /* --help */
	      show_usage (*argv);
	      return 0;
	      break;
	      
	    default:
	      break;
	    }
	  break;
	  
	case 'g':
	  gamma = true;
	  break;
	    
	case 'h':
	  show_usage (*argv);
	  return 0;
	  break;
	  
	  
	case 'o':
	  if (strncmp (optarg, "-", MAXPATHLEN) == 0)
	    {
	      use_stdout = true ;
	    }
	  else
	    {
	      if ( strlen(optarg) < MAXPATHLEN )
		{
                  char filepath[MAXPATHLEN];
		  strncpy(filepath, optarg, MAXPATHLEN);
		  out_filepath = filepath ;
		  use_stdout = false ;
		}
	    }
	  break;

	case 'p':
	  /* select alternate test pattern */ 
	  gamma = false;
	  testPattern = atoi( optarg );
	  break;
	  
	  
	case 's':
	  /* paper size stuffer */
	  if (!strncmp (optarg, "us", 2) || !strncmp (optarg, "letter", 6))
	    {
	      gWidth = LETTERWIDTH;
	      gHeight = LETTERHEIGHT;
	    }
	  else if (!strncmp (optarg, "legal", 5))
	    {
	      gWidth = LEGALWIDTH;
	      gHeight = LEGALHEIGHT;
	    }
	  else if (!strncmp (optarg, "a4", 2))
	    {
	      gWidth = A4WIDTH;
	      gHeight = A4HEIGHT;
	    }
	  else
	    {
	      fprintf(stderr,"%s: unknown paper size \"%s\"\n", 
		      progname, optarg);
	      exit (0);
	    }
	  break;
#if 0
	case 'v':
	  /* printer model */
      	  if (!strncmp (optarg, "820", 3) || !strncmp (optarg, "1000", 4))
	    black_maxlines = 128;
	  else if (!strncmp (optarg, "71", 2) || !strncmp (optarg, "72", 2))
	    /*	    black_maxlines = 300; */
	    black_maxlines = 128;   /*reverted for the moment - Duncan */
	  else
	    {
	      fprintf(stderr,"%s: unknown printer model \"%s\"\n", 
		      progname, optarg);
	      exit (0);
	    }
	  break;
#endif
	  
	default:
	  exit (0);
	  break;
	}
    }
  

  if (use_stdout)
    outFile  =  stdout;
  else
    {
      outFile = fopen( out_filepath , "w");
      fprintf(stderr,"%s: writing requested PPM data to \"%s\"\n", 
	      progname, out_filepath);
    }
      
  if (outFile == NULL )
    {
      fprintf(stderr,"%s: cannot open output file \"%s\"\n", 
	      progname, out_filepath);
      return 0;
    }
  
  if ( gamma )
    {
      /* production of PixMap  gamma.ppm */
      make_ppm_file( outFile , &image, progname );
    }
  else
    {
      /* make PixMap test patterns for printer */
      make_test_pattern( outFile,  testPattern, progname ) ;
    }

  
  fclose (outFile);
  return  1 ;

}

int
make_ppm_file (FILE  *outFile , image_t *image , char * progname)
{
  unsigned char *line = NULL ;
  image->width = gWidth;       
  image->height = gHeight;
  image->bufferCurLine = 0;
  
    fprintf (outFile,
	   "P6\n#PPM (PixMap) Image for comparison with printed output of \"pnm2ppa -g\"\n%d %d\n255\n",
	   image->width, 64 + image->height / 20);

    line = malloc (image->width * 3);
     if (!(line))
      {
	fprintf(stderr,"%s:  malloc failed\n",
		progname);
	exit (1);
      }
    else
      {
	while  ( image->bufferCurLine < image->height)
	  {
	    if ( ProduceGamma (image, line ))
	      {
		fwrite (line, sizeof (unsigned char), (image->width * 3), 
			outFile);
	      }
	    image->bufferCurLine++;
	  }
	free (line);
      }
  return 1;
}


/* various test patterns */

void centering_grid( void );
void make_big_x( void );
void make_vertical_line( void );
void color_alignment( void );
void check_alignment( void );
void head_cleaning( void );
void color_squares( void );
void centering( void );

/* color coding scheme used */

/* each color is coded on 2 bits of 1 byte rrggbbxx 
   (light grey is for head cleaning) 
*/
enum {
  WHITE   =    0xfc,    /*  white       11 11 11 00  = 000 = (255,255,255) */
  BLACK   =    0x00,    /*  black       00 00 00 00  = 444 = (000,000,000) */
  CYAN    =    0xbc,    /*  cyan        10 11 11 00  = 100 = (128,255,255) */
  MAGENTA =    0xec,    /*  magenta     11 10 11 00  = 010 = (255,128,255) */
  YELLOW  =    0xf8,    /*  yellow      11 11 10 00  = 001 = (255,255,128) */
  RED     =    0xc0,    /*  red         11 00 00 00  = 044 = (255,000,000) */
  GREEN   =    0x30,    /*  green       00 11 00 00  = 404 = (000,255,000) */
  BLUE    =    0x0c,    /*  blue        00 00 11 00  = 004 = (000,000,255) */
  GREY    =    0xa8     /*  light grey  10 10 10 00  = 111 = (128,128,128) */
  };


int 
make_test_pattern (FILE *outFile, int testPattern , char * progname )
{
  int x;
  int  line, maxlines = 128 ;

  /* create the pattern in strips starting at line line, 
     with a maximum number of lines maxlines in the strip.
  */

  Width = 8*(gWidth/8);  /* make Width a multiple of 8 */
  Height = gHeight;
  
  /* pixel dimensions */
#if 0
   Pwidth = Width ;
   Pheight = Height ;
#endif

  /* each color is coded on 2 bits if 1 byte rrggbbxx */

      fprintf(outFile,"P6\n# pnm2ppa calibration image\n%d %d\n255\n",
	      Width,Height);

  for ( line = 0; line < Height ; line += maxlines )
    {
      BitMap_topline = line;
      BitMap_bottomline = line + maxlines; 
      if  ( BitMap_bottomline > Height)
	BitMap_bottomline = Height;
      BitMap_Height = BitMap_bottomline - BitMap_topline;
      BitMap_Width = Width;

      color_bitmap = (char *)malloc(BitMap_Width*BitMap_Height);
      

      /* set strip to white */
      memset(color_bitmap,0xff,(BitMap_Width*BitMap_Height));
      
      
      switch(testPattern)
	{
  case 0:
    color_alignment();
    break;
	case 1:
	  check_alignment();
    break;
	case 2:
	  head_cleaning();
	  break;
	case 3:
	  centering();
	  break;
#if 0
	case 4:
	  centering_grid();
	  break;
	case 4:
	  color_squares();
	  break;
	case 5:
	  make_vertical_line();
	  break;
	case 6:
	  make_big_x();
	  break;
#endif	  
	default:
	  fprintf(stderr,"%s: unknown test pattern (%d)\n",progname,testPattern);
	  fprintf(stderr,"test pattern 0 : cartridge alignment (default)\n");
	  fprintf(stderr,"             1 : check alignment\n");
	  fprintf(stderr,"             2 : colored stripes, use for head cleaning\n");
	  fprintf(stderr,"             3 : center paper \n");
	  return 1;
	}
      
            
      for(x=0; x<BitMap_Width*BitMap_Height; x++)
	{
	  int v, mask, i;
	  
	  /* extract the data from the first 6 bits of each byte 
	     of color_bitmap, convert to ppm format, and output it.
	     Each color has 4 intensities, coded in 2 bits): 
	     0x00 : 4 drops of ink - ppm value 0
	     0x01 : 3 drops of ink - ppm value 8
	     0x02 : 1 drop  of ink - ppm value 128
	     0x03 : 0 drops of ink - ppm value 255
	  */
	  
	  for(i=6 ; i > 0 ; i-=2 )
	    {
	      mask = ( 0x03 << i );
	      
	      v = ((color_bitmap [x] & mask) >> i);  
	      switch(v)
		{
		case 0x00: /* 4 drops of ink */
		  v=0x00;
		  break;
		case 0x01: /* 3 drops of ink */
		  v=0x08;
		  break;
		case 0x02: /* 1 drops of ink */
		  v=0x80;
		  break;
		case 0x03: /* 0 drops of ink */
		  v=0xff;
		}
	      putc(v, outFile);
	    }
	}
      free(color_bitmap);
    } 
  return 0;
}





void
centering_grid( void )
{
/* this  pattern is a grid that can be used for centering the page*/
  int x,y;
  char buf[128];


    for(x=0; x<Width; x+=100)
      for(y=0; y<Height; y+=100)
	setplus(x,y,4);
    for(x=0; x<Width; x+=100)
    {
      sprintf(buf,"%d",x);
      setstring(x+3,(Height/200)*100+3,buf,3);
    }
    for(y=0; y<Height; y+=100)
    {
      sprintf(buf,"%d",y);
      setstring((Width/200)*100+3,y+3,buf,3);
    }
    for(x=0; x<Width; x+=10)

      for(y=0; y<Height; y+=100)
	setplus(x,y,((x%100)==50) ? 2 : 1);
    for(x=0; x<Width; x+=100)
      for(y=0; y<Height; y+=10)
	setplus(x,y,((y%100)==50) ? 2 : 1);
    setCG(Width/2,Height/2);
}


void
make_vertical_line( void )
{
  int y;
    for(y=0; y<300; y++)
      setpixel(Width/2,Height/2-y,BLACK);
}


void 
make_big_x( void )
{
  int y;
  for(y=0; y<300; y++)
    {
      setpixel(y,y,BLACK);
      setpixel(Width-1-y,Height-1-y,BLACK);
    }
}







void 
color_alignment( void )
{
  /* add for color test, alignment  pattern */
  int x,y,t,shear,direction;
  int vpos;
  int hpos;
  char buf[128];

  /* horizontal  black/color offset pattern */

  vpos=300;
  hpos=Width/2-2000;
  for(y=1;y<20;y++)
    {
      for(x=0;x<128;x++)
      {
        setpixel(y*200+hpos,x+vpos,BLACK);
        setpixel(y*201-10+hpos,x+vpos+128,BLUE);    
	setpixel(y*201-10+1+hpos,x+vpos+128,BLUE);  
        setpixel(y*200+hpos,x+vpos+256,BLACK);
      }
      if ((y-10) > 0)
	sprintf(buf,"+%d",y-10);
      else
	sprintf(buf,"%d",y-10);
      setstring(y*200+hpos,vpos+400,buf,5);
    }
    setstring(Width/2 -200,vpos+500,"COLOFFSX",7);
    /* vertical black/color offset pattern */

    vpos=1200;
    hpos=Width/2-2150;
    for (t=1;t<10;t++)
      {
     for(y=0;y<101;y++)
       {
	 setpixel(t*400+hpos+y,vpos,BLACK);
        setpixel(t*400+hpos+105+y,vpos-5+t*1,BLUE); 
        setpixel(t*400+hpos+105+y,vpos-5+t*1+1,BLUE); 
        setpixel(t*400+hpos+210+y,vpos,BLACK);
     }  
     if ((t-5) > 0)
       sprintf(buf,"+%d",t-5);
     else
       sprintf(buf,"%d",t-5);
     setstring(t*400+hpos+150,vpos+50,buf,5);
     
      }
    setstring(Width/2-200,vpos+150,"COLOFFSY",7);


    /* colorshear setting (left-to-right / right-to-left sweep offsets) */
 	

    hpos=Width/2-2000;
    vpos=2000;
    direction = 1;   /* this is determined by whether sweep is L2R or R2L */
    for(y=1;y<20;y++)
      {
	shear= direction*(y-10);
	for(x=0;x<128;x++)
	  {
	    setpixel(y*200+hpos,x+vpos+000,BLUE);
	    setpixel(y*200+hpos+1,x+vpos+000,BLUE);
	    setpixel(y*200+hpos+shear,x+vpos+128,BLUE);
	    setpixel(y*200+hpos+1+shear,x+vpos+128,BLUE);
	    setpixel(y*200+hpos,x+vpos+256,BLUE);	
	    setpixel(y*200+hpos+1,x+vpos+256,BLUE);	
	    setpixel(y*200+hpos+shear,x+vpos+384,BLUE);
	    setpixel(y*200+hpos+1+shear,x+vpos+384,BLUE);
	    setpixel(y*200+hpos,x+vpos+512,BLUE);	
	    setpixel(y*200+hpos+1,x+vpos+512,BLUE);	
	  }
	if ((y-10) > 0)
	  sprintf(buf,"+%d",y-10);
	else
	  sprintf(buf,"%d",y-10);
	setstring(y*200+hpos,vpos+650,buf,5);
      }
    setstring(Width/2-200,vpos+750,"COLORSHEAR ",7);
    
	

    /* blackshear setting (left-to-right / right-to-left sweep offsets) */

    hpos=Width/2-2000;
    vpos=3500;
    direction = -1;   /* this is determined by whether sweep is L2R or R2L */
    for(y=1;y<20;y++)
      {   
	shear = -direction*(y-10);
	for(x=0; x < black_maxlines ;x++)
	  {
	    setpixel(y*200+hpos,x+vpos+000,BLACK);
	    setpixel(y*200+hpos+shear,x+vpos+black_maxlines,BLACK);
	    setpixel(y*200+hpos,x+vpos+2*black_maxlines,BLACK);	
	    setpixel(y*200+hpos+shear,x+vpos+3*black_maxlines,BLACK);
	    setpixel(y*200+hpos,x+vpos+4*black_maxlines,BLACK);	
	  }
	if ((y-10) > 0)
	  sprintf(buf,"+%d",y-10);
	else
	  sprintf(buf,"%d",y-10);
	setstring(y*200+hpos,vpos+650+5*(black_maxlines-128),buf,5);
	
      }
    vpos = vpos + 5*(black_maxlines -128) ;
    setstring(Width/2-200,vpos+750,"BLACKSHEAR ",7);
    
    hpos=Width/2;
    vpos=4800;
    for(x=0;x<640;x++)
      {
	setpixel(hpos,vpos+x,BLACK);
	setpixel(hpos,vpos+640+x,BLUE);
      }
}

void 
check_alignment( void )
{
  /* alignment verification pattern */
  int x,y;
  int vpos, vpos1, vpos2;
  int hpos, hpos1,hpos2;


  /* cross */

   vpos=1000;
   hpos=Width/2-384;
   hpos+=384;
   for (x=0;x<black_maxlines-2;x++)
       setpixel(hpos,vpos+x,BLACK);
   vpos += black_maxlines;
   for (x=0;x<126;x++)
       setpixel(hpos,vpos+x,BLUE);
   vpos  += 128;
   for (x=0;x<black_maxlines-2;x++)
       setpixel(hpos,vpos+x,BLACK);
   vpos += black_maxlines;
   for (x=0;x<black_maxlines-2;x++)
       setpixel(hpos,vpos+x,BLACK);
   vpos += black_maxlines;
   for (x=0;x<126;x++)
       setpixel(hpos,vpos+x,BLUE);
   vpos  += 128;
   for (x=0;x<black_maxlines-2;x++)
       setpixel(hpos,vpos+x,BLACK);
   vpos += black_maxlines;

     vpos-=128+2*black_maxlines;
     hpos-=128+2*black_maxlines;

   for (y=0;y<black_maxlines-2;y++)
       setpixel(hpos+y,vpos,BLACK);
   hpos += black_maxlines;
   for (y=0;y<126;y++)
       setpixel(hpos+y,vpos,BLUE);
   hpos  += 128;
   for (y=0;y<black_maxlines-2;y++)
       setpixel(hpos+y,vpos,BLACK);
   hpos += black_maxlines;
   for (y=0;y<black_maxlines-2;y++)
       setpixel(hpos+y,vpos,BLACK);
   hpos += black_maxlines;
   for (y=0;y<126;y++)
       setpixel(hpos+y,vpos,BLUE);
   hpos  += 128;
   for (y=0;y<black_maxlines-2;y++)
       setpixel(y+hpos,vpos,BLACK);


   /* square */

   vpos=3000;
   hpos1=Width/2-3*(128+black_maxlines);
   hpos2=Width/2+3*(128+black_maxlines);
   for (x=0;x<2*black_maxlines -2;x++)
     { 
       setpixel(hpos1,vpos+x,BLACK);     
       setpixel(hpos2,vpos+x,BLACK);     
     }
   vpos += 2*black_maxlines;
   for (x=0;x<254 ;x++)
     {
       setpixel(hpos1,vpos+x,BLUE);
       setpixel(hpos2,vpos+x,BLUE);
     }
   vpos +=256;
   for (x=0;x<2*black_maxlines -2;x++)
     {
       setpixel(hpos1,vpos+x,BLACK);     
       setpixel(hpos2,vpos+x,BLACK);     
     }
   vpos += 2*black_maxlines;
   for (x=0;x<254 ;x++)
     {
       setpixel(hpos1,vpos+x,BLUE);
       setpixel(hpos2,vpos+x,BLUE);
     }
   vpos +=256;
   for (x=0;x<2*black_maxlines -2;x++)
     {
       setpixel(hpos1,vpos+x,BLACK);     
       setpixel(hpos2,vpos+x,BLACK);     
     }
   vpos += 2*black_maxlines;
   for (x=0;x<254 ;x++)
     {
       setpixel(hpos1,vpos+x,BLUE);
       setpixel(hpos2,vpos+x,BLUE);
     }

   vpos1=3000;
   vpos2 = vpos1+6*(128+black_maxlines);
   hpos = Width/2 - 3*(128+black_maxlines);
   for (y=0;y<2*black_maxlines -2;y++)
     {
       setpixel(hpos+y,vpos1,BLACK);     
       setpixel(hpos+y,vpos2,BLACK);     
     }
   hpos += 2*black_maxlines;
   for (y=0;y<254 ;y++)
     {
       setpixel(hpos+y,vpos1,BLUE);
       setpixel(hpos+y,vpos2,BLUE);
     }
   hpos +=256;
   for (y=0;y<2*black_maxlines -2;y++)
     {
       setpixel(hpos+y,vpos1,BLACK);     
       setpixel(hpos+y,vpos2,BLACK);     
     }
   hpos += 2*black_maxlines;
   for (y=0;y<254 ;y++)
     {
       setpixel(hpos+y,vpos1,BLUE);
       setpixel(hpos+y,vpos2,BLUE);
     }
   hpos +=256;
   for (y=0;y<2*black_maxlines -2;y++)
     {
       setpixel(hpos+y,vpos1,BLACK);     
       setpixel(hpos+y,vpos2,BLACK);     
     }
   hpos += 2*black_maxlines;
   for (y=0;y<254 ;y++)
     {
       setpixel(hpos+y,vpos1,BLUE);
       setpixel(hpos+y,vpos2,BLUE);
     }
}

void
centering(void)
{
  int x,y,z;
  char buf[128];

  for (y = Width/2; y > 0;  y-- )
    { 
      setpixel(y,y, BLACK);
      setpixel(Width-y,y, BLACK);
      setpixel(y,Height-y, BLACK);
      setpixel(Width-y,Height-y, BLACK);
      setpixel(y,Height/2, BLACK);
      setpixel(Width-y,Height/2, BLACK);
    }
  
  for (x = 0; x < Height; x++)
     setpixel(Width/2,x, BLACK );

  for (z = 50; z < 601; z += 50)
  {
    for (x = z; x <=  Height -z; x++)
      {
	setpixel(z,x,BLACK);
	setpixel(Width-z,x,BLACK);
      }
    for (y = z; y <= Width -z;y++)
      {
	setpixel(y,z,BLACK);
	setpixel(y,Height-z,BLACK);
      }
      sprintf(buf,"%d",z);
      setstring (Width/2-30,z-15,buf,5);
      setstring(Width/2-30,Height-z-15,buf,5);
      setstringv(z-15,Height/2-60,buf,5);
      setstringv(Width-z-15,Height/2-60,buf,5);
      
  }
  setstring(700,Height/2-100,"XOFFSET",7);
  setstring(Width/2-150,800,"YOFFSET",7);

}


void 
head_cleaning( void )
{
  /* test page for cleaning */
  int x,y,a,b,c;
  int vpos;
  char buf[128];

  vpos=100;	  
  for (y=0;y<2500;y++) 
    {
      a=y%250 ;
      b=y/50 ;
      if (a==0) 
	{
	  for (x=0;x<300;x++)
            {
	      setpixel ( y+100,x+vpos,BLACK);
            }
	}
      else
	{
	  for (x=0;x<6;x++)
	    {
	      a=x*50+b*1 ;
	      if (a>300) a=a-300;
	      setpixel (y+100,a+vpos,BLACK);
	    }
	}
    }
  for (a=50;a<301;a+=50)
    { 
      sprintf (buf,"%d",a);
      setstring (2510+100,vpos-50+a,buf,3);
    }
  
  for (y=0;y<(240*8);y++) 
    {
      a=y%240 ;
      
      b=y/30 ;
      vpos=500;
      if (a==0) 
	{
	  for (x=0;x<128;x++)
            {
	      setpixel ( y+100,x+vpos,MAGENTA);
	      setpixel ( y+100,x+vpos+128,YELLOW);
	      setpixel ( y+100,x+vpos+256,CYAN);
            }
	}
      else
	{
	  for (x=0;x<1;x++)
	    {
	      a=x*50+b*2 ;
		if (a>128) a=a-128;
                setpixel (y+100,a+vpos,MAGENTA);
                setpixel (y+100,a+vpos+128,YELLOW);
                setpixel (y+100,a+vpos+256,CYAN);
	    }
	}
    }

  vpos=1300;
  for (y=0;y<3500;y++)
    {
      a=y/500;
      
      switch (a)
        {
        case 0 :  b = RED; break;
        case 1 :  b = MAGENTA; break;
        case 2 :  b = BLUE; break;
        case 3 :  b = CYAN; break;
        case 4 :  b = GREEN; break;
        case 5 :  b = YELLOW; break;
        case 6 :  b = RED; break;
        }
      for (x=0;x<128;x++)
        {
	  setpixel (y+100,x+vpos,b);
        }  
      }
  
  /*grey*/
  vpos=2000;
  for (y=100;y<Width-1000;y++)
      {
	for (x=0;x<128;x++)
	  {
	    setpixel (y,x+vpos,GREY);
	  }
      }
  
  /*magenta,yellow,cyan*/
  vpos=2300;
  for (y=100;y<Width-1000;y++)
    {
      for (x=0;x<128;x++)
        {
        setpixel (y,x+vpos,MAGENTA);
        setpixel (y,x+vpos+128,YELLOW);
        setpixel (y,x+vpos+256,CYAN);
        }
      }
  /*black*/
  vpos=2800;
  for (y=100;y<Width-1000;y++)
    {
      for (x=0;x<300;x++)
        {
	  setpixel (y,x+vpos,BLACK);
        }
    }
  
  vpos=3800;
  for (y=0;y<600;y++) 
    {
      a=y%50 ;
      b=y/50 ;
      c=y/100;
      if (a==0) 
	{
	  for (x=0;x<300;x++)
            {
	      setpixel ( y+100,x+vpos,BLACK);
            }
	}
      else
          {
	    for (x=0;x<25;x++)
	      {
		a=x*2+c*50 ;
                a=a+(b%2);
                setpixel (y+100,a+vpos,BLACK);
	      }
          }
    }
  

  for (y=600;y<1701;y++) 
     {
       a=y%50 ;
       b=y/50 ;
       if (a==0) 
         {
           for (x=0;x<300;x++)
            {
	      setpixel ( y+100,x+vpos,BLACK);
            }
	 }
        else
          {
	    for (x=0;x<7;x++)
	      {
		        a=(x*50+b*2)%300 ;
			c=(a+3)%300;
			setpixel (y+100,a+vpos,BLACK);
			setpixel (y+100,c+vpos,BLACK);
	      }
          }
     }
  
  for (y=1700;y<2400;y++) 
      {
	a=(y-1700)/70 ;
	for (x=0;x<30;x++)
		  {
		    setpixel (y+100,vpos+x*10+a,BLACK);
		  }
      } 

  for (y=2400;y<3100;y++) 
    {
       a=(y-2400)%70 ;
       if (a==0) 
         {
           for (x=0;x<300;x++)
	     {
               setpixel ( y+100,x+vpos,BLACK);
	     }
	 }
	 }
} 


void 
color_squares( void )
{
  int x,y,z;
  /* color squares  (Giorgio) (dont work after modifications 15/03/2000)
     seems to use a different color coding scheme ..... 125 possible
     color values in a byte ... */


    z=0;
    for (x=0;x<5;x++)
      for (y=0;y<5;y++)
	{
	  setsquare( 300*(x+1), 300+300*(y),200, 1+x,1+y,1+z);
	  setchar( 300*(x+1), 270+300*(y), '0' +  x,3);
	  setchar( 40+300*(x+1), 270+300*(y), '0' +  y,3);
	  setchar( 80+300*(x+1), 270+300*(y), '0' +  z,3);
	}
    z=1;
    for (x=0;x<5;x++)
      for (y=0;y<5;y++)
	{
	  setsquare( 1500+300*(x+1), 300+300*(y),200, 1+x,1+y,1+z);
	  setchar( 1500+300*(x+1), 270+300*(y), '0' +  x,3);
	  setchar( 1540+300*(x+1), 270+300*(y), '0' +  y,3);
	  setchar( 1580+300*(x+1), 270+300*(y), '0' +  z,3);
	}
    z=2;
    for (x=0;x<5;x++)
      for (y=0;y<5;y++)
 	{
 	  setsquare( 3000+300*(x+1), 300+300*(y),200, 1+x,1+y,1+z);
	  setchar( 3000+300*(x+1), 270+300*(y), '0' +  x,3);
	  setchar( 3040+300*(x+1), 270+300*(y), '0' +  y,3);
	  setchar( 3080+300*(x+1), 270+300*(y), '0' +  z,3);
	}
    z=3;
    for (x=0;x<5;x++)
      for (y=0;y<5;y++)
	{
	  setsquare( 300*(x+1), 1800+300*(y),200, 1+x,1+y,1+z);
	  setchar( 300*(x+1), 1770+300*(y), '0' +  x,3);
	  setchar( 40+300*(x+1), 1770+300*(y), '0' +  y,3);
	  setchar( 80+300*(x+1), 1770+300*(y), '0' +  z,3);
	}
    z=4;
    for (x=0;x<5;x++)
      for (y=0;y<5;y++)
	{
	  setsquare( 1500+300*(x+1), 1800+300*(y),200, 1+x,1+y,1+z);
	  setchar( 1500+300*(x+1), 1770+300*(y), '0' +  x,3);
	  setchar( 1540+300*(x+1), 1770+300*(y), '0' +  y,3);
	  setchar( 1580+300*(x+1), 1770+300*(y), '0' +  z,3);
	}
}

