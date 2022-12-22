/*****************************************************************
     make_hash_ink.h -generate hashed version of palette.h data
                              -----------------
     begin          : Tue Jan 24 2000
     copyright      : (C) 2000 by the pmn2ppa project
     author         : Klamer Schutte
     email          : Schutte@fel.tno.nl
******************************************************************/

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
#include <assert.h>

#include "palette.h"
#include "hash_ink.h"

#define SHIFTED         (1 << (shift))
#define NOTSHIFTED      (1 << (8 - shift))
#define ENTRIES_ARR     (NOTSHIFTED * NOTSHIFTED * NOTSHIFTED)

int MaxPass;

struct entry
{
  int xyz;
  union {
    int entry_nr;
    struct hash_ink *hi;
  } u;
  struct entry *next; /* this list is sorted on xyz; lowest xyz first */
};

struct entry_list
{
  struct entry *entries;
  int num_entries; 
  struct entry_list *next; /* this list is sorted in num_entries; lowest
			      numbers first */
} *start_list = 0;

inline int 
calc_xyz( struct hash_ink *hi )
{ 
  /* if these assertions fail the result is not unique */
  assert(hi->x < (1 << 3));
  assert(hi->y < (1 << 3));

  return hi->x + (hi->y << 3) + (hi->z << 6);
}

int 
add_entry2( struct entry *e, int n, struct entry **ptr_to )
{
  struct entry_list *cel, **prev_ptr, *new_el;
  struct entry *ce1, *ce2;
  static int this_entry = 0;
  int next_entry;

  assert(n > 0);

  prev_ptr = &start_list;
  for(cel = start_list; cel != 0; prev_ptr = &(cel->next), cel = cel->next)
    {
       if (cel->num_entries < n)
	continue;

      if (cel->num_entries > n)
	break;

      for(ce1 = cel->entries, ce2 = e; ce1 != 0; 
	                         ce1 = ce1->next, ce2 = ce2->next)
	{
	  assert(ce2 != 0);
	  
	  if (ce1->xyz != ce2->xyz)
	    goto next_cel;
	}
      /* OK, we have found the correct entry. Delete set *e, and
	 return the entry number */
      for(ce1 = e; ce1 != 0; ce1 = ce2)
	{
	  ce2 = ce1->next;
	  free(ce1);
	}
      
      if (ptr_to != 0)
	*ptr_to = cel->entries;
      return cel->entries->u.entry_nr;

    next_cel: ;
    }

  /* Hmmm. The set e is not found. Add the first item to the set,
     and recursively attempt it for its tail... */

  new_el = (struct entry_list *) malloc(sizeof(struct entry_list));
  /* printf(" New el %x assigned to %x\n", new_el, prev_ptr); */
  *prev_ptr = new_el;
  new_el->entries = e;
  new_el->num_entries = n;
  /* printf("new el next = %x\n", cel); */
  new_el->next = cel;

  if (e->next != 0)
    next_entry = add_entry2( e->next, n-1, &(e->next));
  else
    next_entry = -1;
  
  printf("\t/* %d */\t{ { %d, %d, %d}, %d, %d, %d, ", this_entry, 
	 e->u.hi->data[0], e->u.hi->data[1], e->u.hi->data[2], 
	 e->u.hi->x, e->u.hi->y, e->u.hi->z );
  if (next_entry == -1)
    printf("0 },\n");
  else
    printf("&entries%d[%d] },\n", MaxPass, next_entry);

  e->u.entry_nr = this_entry++;
  if (ptr_to != 0)
    *ptr_to = e;
  return e->u.entry_nr;
}

int
add_entry( struct hash_ink *hi )
{
  /* First, convert the hash_ink set to a entry set
     Second, check start_list if it is contained (done in add_entry2())
        If contained: return the entry number
        Else: add to start_list */

  struct entry *first = 0, *cur, *loop, **prev_ptr;
  struct hash_ink *chi;
  int res;
  int n = 0; /* n == number of entries in this set */

  for(chi = hi; chi != 0; chi = chi->next)
    {
      n++;
      cur = (struct entry *) malloc( sizeof(struct entry) );
      cur->xyz = calc_xyz( chi );
      cur->u.hi = chi;
      prev_ptr = &first;
      for(loop = first; loop != 0; loop = loop->next)
	{
	  if (loop->xyz >= cur->xyz)
	    {
	      cur->next = loop;
	      *prev_ptr = cur;
	      break;
	    }
	  prev_ptr = &(loop->next);
	}
      if (loop == 0)
	{
	  cur->next = 0;
	  *prev_ptr = cur;
	}
    }

  res = add_entry2( first, n, 0 );

  for(chi = hi; chi != 0; )
    {
      struct hash_ink *nhi = chi->next;
      free(chi);
      chi = nhi;
    }

  return res;
}
  
int 
main(int argc, char *argv[] )
{
  /* int MaxPass; */
  int x, y, z, r, g, b;
  int ra, ga, ba, rd, gd, bd;
  float dmin = -1, dmax = -1, d1;
  struct hash_ink /* **arr, */ *first, *prev, *cur;
  int i, n;
  int shift;
  int *entries_arr;

  if (argc != 3)
    fprintf(stderr, "Usage: %s MaxPass shift >> hash_ink.c\n", argv[0]);

  MaxPass = atoi(argv[1]);
  shift = atoi(argv[2]);

  /* arr = (struct hash_ink **) malloc( sizeof(struct hash_ink*) * ENTRIES_ARR ); 
  for(i = 0; i < ENTRIES_ARR; i++)
    arr[i] = 0; */
  entries_arr = (int *) malloc(sizeof(int) * ENTRIES_ARR);
  for(i = 0; i < ENTRIES_ARR; i++)
    entries_arr[i] = -1;

  printf("/* generated by %s %s %s */\n\n", argv[0], argv[1], argv[2] );
  printf("#include \"hash_ink.h\"\n\n");
  printf("int shift%d = %d;\n\n", MaxPass, shift);
  printf("static struct hash_ink entries%d[] = {\n", MaxPass);

  i = 0;

  for(r=0; r < 256; r += SHIFTED)
    for(g=0; g < 256; g += SHIFTED)
      for(b=0; b < 256; b += SHIFTED)
	{
	  int rr = r >> shift;
	  int gg = g >> shift;
	  int bb = b >> shift;
	  int h = (rr << (16 - shift * 2)) + (gg << (8-shift)) + bb;
/* fprintf(stderr, "%d %d %d -> %d %d %d -> %d\n", r, g, b, rr, gg, bb, h );*/
	  first = (struct hash_ink *) malloc( sizeof(struct hash_ink));
	  /* arr[h]->x = i++; */
	  cur = first;
	  cur->next = 0;
	    
	  /* Now, calculate the dmin for hash entry h.
	     Fill in *first with minimal values */
	  dmin = -1;
	  for(ra = 0; ra < SHIFTED; ra+=(SHIFTED-1))
	    for(ga = 0; ga < SHIFTED; ga += (SHIFTED-1))
	      for(ba = 0; ba < SHIFTED; ba += (SHIFTED -1))
		{
		  int R = r + ra;
		  int G = g + ga;
		  int B = b + ba;
		  int new_dmin = 0;
		  
		  for (z = 0; z < MaxPass + 1; z++)
		    for (x = 0; x < MaxPass + 1; x++)
		      for (y = 0; y < MaxPass + 1; y++)
			if (!
			    (((x == 4) && (y > 2) && (z > 2)) ||
			     ((y == 4) && (x > 2) && (z > 2)) ||
			     ((z == 4) && (y > 2) && (x > 2))))
			{
			  rd =
			    (int) R - (int) hp_pal[4 - x][4 - y][4 -
								z]
			    [0];
			  gd =
			    (int) G - (int) hp_pal[4 - x][4 - y][4 -
								z]
			    [1];
			  bd =
			    (int) B - (int) hp_pal[4 - x][4 - y][4 -
								z]
			    [2];
			  
			  d1 = /* 0.5 * */ rd * rd + gd * gd + bd * bd;

			  if ((dmin < 0) || (d1 < dmin))
			    {
			      /* fprintf(stderr, "Found min %f, %d, %d, %d (%f, %d, %d %d)\n", d1, x, y, z, dmin, cur->x, cur->y, cur->z); */
			      cur->data[0] = hp_pal[4 - x][4 - y][4 - z][0];
			      cur->data[1] = hp_pal[4 - x][4 - y][4 - z][1];
			      cur->data[2] = hp_pal[4 - x][4 - y][4 - z][2];
			      cur->x = x;
			      cur->y = y;
			      cur->z = z;
			      dmin = d1;
			      new_dmin = 1;
			    }
			}
		  
		}

	  /* Now, calculate the dmax for hash entry h.
	     Fill in *first with minimal values */
	  dmax = -1;
	  for(ra = 0; ra < SHIFTED; ra+=(SHIFTED-1))
	    for(ga = 0; ga < SHIFTED; ga += (SHIFTED-1))
	      for(ba = 0; ba < SHIFTED; ba += (SHIFTED-1))
		{
		  int R = r + ra;
		  int G = g + ga;
		  int B = b + ba;
		  
		  z = cur->z;
		  y = cur->y;
		  x = cur->x;
			{
			  rd =
			    (int) R - (int) hp_pal[4 - x][4 - y][4 -
								z]
			    [0];
			  gd =
			    (int) G - (int) hp_pal[4 - x][4 - y][4 -
								z]
			    [1];
			  bd =
			    (int) B - (int) hp_pal[4 - x][4 - y][4 -
								z]
			    [2];
			  
			  d1 = /* 0.5 * */ rd * rd + gd * gd + bd * bd;

			  if (d1 > dmax)
			    {
			      /* fprintf(stderr, "Found max %f, %d, %d, %d (%f, %f)\n", d1, x, y, z, dmax, dmin); */
			      cur->data[0] = hp_pal[4 - x][4 - y][4 - z][0];
			      cur->data[1] = hp_pal[4 - x][4 - y][4 - z][1];
			      cur->data[2] = hp_pal[4 - x][4 - y][4 - z][2];
			      cur->x = x;
			      cur->y = y;
			      cur->z = z;
			      dmax = d1;			     
			    }
			}
		}
	  
	  /* Now, for every {x,y,z} for which d1 is smaller then dmax add
	     this entry to the *first (as a linked list!) */
	  prev = cur;
	  n = 0;
	  for (z = 0; z < MaxPass + 1; z++)
	    for (x = 0; x < MaxPass + 1; x++)
	      for (y = 0; y < MaxPass + 1; y++)
		{
		if (!
		    (((x == 4) && (y > 2) && (z > 2)) ||
		     ((y == 4) && (x > 2) && (z > 2)) ||
		     ((z == 4) && (y > 2) && (x > 2))))
		  {
		    for(ra = 0; ra < SHIFTED; ra+=(SHIFTED-1))
		      for(ga = 0; ga < SHIFTED; ga += (SHIFTED-1))
			for(ba = 0; ba < SHIFTED; ba += (SHIFTED-1))
			  {
			    int R = r + ra;
			    int G = g + ga;
			    int B = b + ba;

			    rd =
			      (int) R - (int) hp_pal[4 - x][4 - y][4 -
								  z]
			      [0];
			    gd =
			      (int) G - (int) hp_pal[4 - x][4 - y][4 -
								  z]
			      [1];
			    bd =
			      (int) B - (int) hp_pal[4 - x][4 - y][4 -
								  z]
			      [2];
			  
			    d1 = /* 0.5 * */ rd * rd + gd * gd + bd * bd;
			    
			    if ((d1 < dmax) && (!((x == first->x) && 
						  (y == first->y) &&
						  (z == first->z))))
			      {
				struct hash_ink *cur = (struct hash_ink *)
				  malloc( sizeof(struct hash_ink) );
				prev->next = cur;
				cur->next = 0;
				prev = cur;
				
				/* fprintf(stderr, "Added min %f, %d, %d, %d (%f, %f, %d, %d %d)\n", d1, x, y, z, dmin, dmax, first->x, first->y, first->z); */
				
				cur->data[0] = hp_pal[4 - x][4 - y][4 - z][0];
				cur->data[1] = hp_pal[4 - x][4 - y][4 - z][1];
				cur->data[2] = hp_pal[4 - x][4 - y][4 - z][2];
				cur->x = x;
				cur->y = y;
				cur->z = z;
				n++;
				goto next_xyz; /* break out of ra gb ba loops */
			      }
			  }
		  }
		  next_xyz: ;
		}
	  /* fprintf(stderr, "%d %d %d (%d): found %d %d %d (%d,%d,%d) (%f, %f) * %d\n",
		  r, g, b, h, first->x, first->y, first->z,
		  first->data[0], first->data[1], first->data[2],
		  dmin, dmax, n ); */
	  entries_arr[h] = add_entry(first);
	}

#ifdef notdef
  printf("#include \"hash_ink.h\"\n\n");
  printf("int shift%d = %d;\n\n", MaxPass, shift);
  printf("static struct hash_ink entries%d[] = {\n", MaxPass);
  
  n = 0;
  for(i = 0; i < ENTRIES_ARR; i++)
    {
      printf("\t{ {%d, %d, %d}, %d, %d, %d, ", 
	     arr[i]->data[0], arr[i]->data[1], arr[i]->data[2], 
	     arr[i]->x, arr[i]->y, arr[i]->z );
      n++;
      cur = arr[i]->next;
      while( cur != 0 )
	{
	  printf(" &entries%d[%d] },\n", MaxPass, n);
	  printf("\t\t{ { %d, %d, %d} , %d, %d, %d, ", 
		 cur->data[0], cur->data[1], cur->data[2], 
		 cur->x, cur->y, cur->z );
	  n++;
	  cur = cur->next;
	}
      printf(" 0 },\n");
    }
#endif
  printf("};\n\n");
  
  printf("struct hash_ink *arr_max%d[] = {", MaxPass);

  n = 0;
  for(i = 0; i < ENTRIES_ARR; i++)
    {
      printf("\t&entries%d[%d], \n", MaxPass, entries_arr[i]);
#ifdef notdef
      n++;
      cur = arr[i]->next;
      while( cur != 0)
	{
	  n++;
	  cur = cur->next;
	}
#endif
    }
  
  printf("};\n\n/* end of file generated by %s %s %s (%d entries) */\n\n", 
	 argv[0], argv[1], argv[2], n);

  return 0;
}
