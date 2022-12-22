/***************************************************************************
    pnmtools.c  -  routines for creating pnm images. 
                             -------------------
    begin                : Sat Oct 14 2000
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

#include <math.h>
#include "pnmtools.h"



/* setpixel(x,y,color) writes a pixel with bytecode color at (x,y)*/
void 
setpixel(int x,int y, unsigned char color )
{
  int charidx;
  if( y < BitMap_topline || y >= BitMap_bottomline ) return;
  if( x < 0 || x >= BitMap_Width   ) return;
  charidx = (y-BitMap_topline)*BitMap_Width+x;
  color_bitmap[charidx] = color ;
}



/*setplus(x,y,s) writes a black "+" of size s centered at (x,y) */
void 
setplus(int x,int y,int s)
{
  int i;

  for(i=0; i<s; i++)
  {
    setpixel(x+i,y,1);
    setpixel(x-i,y,1);
    setpixel(x,y+i,1);
    setpixel(x,y-i,1);
  }
}

/*setx(x,y,s) writes a black "X" of size s centered  (x,y) */
void 
setx(int x,int y,int s)
{
  int i;

  for(i=0; i<s; i++)
  {
    setpixel(x+i,y+i,1);
    setpixel(x-i,y+i,1);
    setpixel(x+i,y-i,1);
    setpixel(x-i,y-i,1);
  }
}

/*setblock(x,y,s) writes a solid black square block of size s 
  centered at (x,y) */
void 
setblock(int x,int y,int s)
{
  int i,j;

  for(i=0; i<s; i++)
    for(j=0; j<s; j++)
      setpixel(x+i,y+j,1);
}

/*
  setchar(x,y,c,s) writes a black character c  at (x,y), using blocks
  of size s.  The  supported characters c belong to a 38-character alphabet
  (0-9, A-Z, -, +) 
*/ 
void 
setchar(int x,int y,char c,int s)
{
  int xo,yo;
  static char charmap[38][5]= { { 0x3e, 0x41, 0x41, 0x41, 0x3e },    /* 0 */
				{ 0x00, 0x42, 0x7f, 0x40, 0x00 },    /* 1 */
				{ 0x42, 0x61, 0x51, 0x49, 0x46 },    /* 2 */
				{ 0x22, 0x41, 0x49, 0x49, 0x36 },    /* 3 */
				{ 0x18, 0x14, 0x12, 0x7f, 0x10 },    /* 4 */
				{ 0x27, 0x45, 0x45, 0x45, 0x39 },    /* 5 */
				{ 0x3e, 0x49, 0x49, 0x49, 0x32 },    /* 6 */
				{ 0x01, 0x01, 0x61, 0x19, 0x07 },    /* 7 */
				{ 0x36, 0x49, 0x49, 0x49, 0x36 },    /* 8 */
				{ 0x26, 0x49, 0x49, 0x49, 0x3e },    /* 9 */
				{ 0x7c, 0x12, 0x11, 0x12, 0x7c },    /* A */
				{ 0x7f, 0x49, 0x49, 0x49, 0x36 },    /* B */
				{ 0x3e, 0x41, 0x41, 0x41, 0x22 },    /* C */
				{ 0x7f, 0x41, 0x41, 0x41, 0x3e },    /* D */
				{ 0x7f, 0x49, 0x49, 0x49, 0x41 },    /* E */
				{ 0x7f, 0x09, 0x09, 0x09, 0x01 },    /* F */
				{ 0x3e, 0x41, 0x41, 0x51, 0x32 },    /* G */
				{ 0x7f, 0x08, 0x08, 0x08, 0x7f },    /* H */
				{ 0x00, 0x41, 0x7f, 0x41, 0x00 },    /* I */
				{ 0x61, 0x41, 0x41, 0x7f, 0x01 },    /* J */
				{ 0x7f, 0x08, 0x14, 0x22, 0x41 },    /* K */
				{ 0x7f, 0x40, 0x40, 0x40, 0x40 },    /* L */
				{ 0x7f, 0x02, 0x04, 0x02, 0x7f },    /* M */
				{ 0x7f, 0x06, 0x08, 0x30, 0x7f },    /* N */
				{ 0x3e, 0x41, 0x41, 0x41, 0x3e },    /* O */
				{ 0x7f, 0x09, 0x09, 0x09, 0x0f },    /* P */
				{ 0x3e, 0x41, 0x51, 0x61, 0x7e },    /* Q */
				{ 0x7f, 0x09, 0x19, 0x29, 0x4f },    /* R */
				{ 0x22, 0x45, 0x49, 0x51, 0x22 },    /* S */
				{ 0x01, 0x01, 0x7f, 0x01, 0x01 },    /* T */
				{ 0x3f, 0x40, 0x40, 0x40, 0x3f },    /* U */
				{ 0x07, 0x18, 0x60, 0x18, 0x07 },    /* V */
				{ 0x3f, 0x40, 0x20, 0x40, 0x3f },    /* W */
				{ 0x63, 0x14, 0x08, 0x14, 0x63 },    /* X */
				{ 0x03, 0x04, 0x78, 0x04, 0x03 },    /* Y */
				{ 0x61, 0x51, 0x49, 0x45, 0x43 },    /* Z */
				{ 0x00, 0x08, 0x08, 0x08, 0x00 },    /* - */
				{ 0x08, 0x08, 0x3e, 0x08, 0x08 } };  /* + */


  if(c<='9' && c>='0' )
    for(xo=0; xo<5; xo++)
      for(yo=0; yo<8; yo++)
	if((charmap[c-'0'][xo]>>yo)&1)
	  setblock(x+xo*s,y+yo*s,s);
  if(c<='Z' && c>='A')
    for(xo=0; xo<5; xo++)
      for(yo=0; yo<8; yo++)
	if((charmap[c-'A'+10][xo]>>yo)&1)
	  setblock(x+xo*s,y+yo*s,s);
  if(c=='-')
    for(xo=0; xo<5; xo++)
      for(yo=0; yo<8; yo++)
	if((charmap[36][xo]>>yo)&1)
	  setblock(x+xo*s,y+yo*s,s);
  if(c=='+')
    for(xo=0; xo<5; xo++)
      for(yo=0; yo<8; yo++)
	if((charmap[37][xo]>>yo)&1)
	  setblock(x+xo*s,y+yo*s,s);
}


/* setstring(x,y,*p,s) writes the string *p  starting
at (x,y), in characters of size s.   The letters of the string
must belong to the alphabet (0-9,A-Z,+,-) supported by setchar() */
void 
setstring(int x,int y,char* s,int size)
{
  char* p;
  int xo;

  for(xo=0, p=s; *p; xo+=((6*size)+1), p++)
    setchar(x+xo,y,*p,size);
}

/* like setstring, but writes vertically downwards*/
void
setstringv(int x,int y,char* s,int size)
{
  char* p;
  int yo;

  for(yo=0, p=s; *p; yo+=((7*size)+1), p++)
    setchar(x,y+yo,*p,size);
}

/* setCG(x,y) writes a "centering symbol" at (x,y) */
void 
setCG(int x,int y)
{
  int xo,yo,zo;

  for(xo=0; xo<=50; xo++)
  {
    yo=sqrt(50.0*50.0-xo*xo);
    setpixel(x+xo,y+yo,1);
    setpixel(x+yo,y+xo,1);
    setpixel(x-1-xo,y-1-yo,1);
    setpixel(x-1-yo,y-1-xo,1);
    setpixel(x+xo,y-1-yo,1);
    setpixel(x-1-xo,y+yo,1);
    for(zo=0; zo<yo; zo++)
    {
      setpixel(x+xo,y-1-zo,1);
      setpixel(x-1-xo,y+zo,1);
    }
  }
}

/* setsquare(x,y,s,r,g,b) looks like it is supposed to
write a solid colored square of size s, bounded by a black
box, "at" (x,y). with "color" (r,g,b).  This is
definitely broken by the color coding scheme used by
setpixel
*/

void 
setsquare(int x, int y, int s, int r, int g, int b)
{
  int i,j;
  int charidx;


  if(x<0 || x>=Width) return;
  if(y<BitMap_topline || y>=BitMap_bottomline) return;

  for(i=0; i<s; i++)
    for(j=0; j<s; j++)
      {
	charidx=(y-BitMap_topline+j)*BitMap_Width+(x+i)*3;
	color_bitmap[charidx] = r;
 	color_bitmap[charidx+1] = g;
 	color_bitmap[charidx+2] = b;
      }
  for(j=0; j<s; j++)
    {
      charidx=(y-BitMap_topline+j)*BitMap_Width+(x)*3;
      color_bitmap[charidx] = color_bitmap[charidx+1] = color_bitmap[charidx+2] = 0;
      charidx+=s*3;
      color_bitmap[charidx] = color_bitmap[charidx+1] = color_bitmap[charidx+2] = 0;
    }
  for(i=0; i<s; i++)
    {
      charidx=(y-BitMap_topline)*BitMap_Width+(x+i)*3;
      color_bitmap[charidx] = color_bitmap[charidx+1] = color_bitmap[charidx+2] = 0;
      charidx+=s*BitMap_Width;
      color_bitmap[charidx] = color_bitmap[charidx+1] = color_bitmap[charidx+2] = 0;
    }
   
}




