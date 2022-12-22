#include <stdio.h>

void parse_scp (FILE *fptr, int length, unsigned long *pos)
{
  int comspec, comref, priority, blank, dlen, i, numbytes;

  printf ("  ** Encapsulated SCP packet found **\n");
  comspec = (getc (fptr) << 8) | getc (fptr);
  comref = (getc (fptr) << 8) | getc (fptr);
  priority = getc (fptr);
  blank = getc (fptr);
  dlen = (getc (fptr) << 8) | getc (fptr);
  *pos += 8;

  printf ("  Command specifier: %d (0x%X)\n", comspec, comspec);

  if (dlen != length - 8 || comspec == 0x186)
    {
      printf ("  ++ Detected HP 720/1000 format\n");
      printf ("  Command reference #: %d\n", dlen);
      dlen = length - 16;

      numbytes = (getc(fptr)<<24) | (getc(fptr)<<16) | (getc(fptr)<<8) |
	getc(fptr);
      getc (fptr);
      getc (fptr);
      getc (fptr);
      getc (fptr);

      *pos += 8;

      printf ("  Data consumed: %d\n", numbytes);
    }
  else
    {
      printf ("  Data length: %d\n", dlen);
      printf ("  Command reference #: %d (0x%X)\n", comref, comref);
    }

  printf ("  Priority: %d\n", priority);

  /* print out data packet */
  printf ("  Data: (pos=%08lx)\n  ", *pos);
  for (i = 0; i < dlen; i++)
    {
      if (i != 0 && i % 16 == 0)
	printf ("\n  ");
      else if (i != 0 && i % 8 == 0)
	printf (" ");

      printf ("%02X ", getc (fptr));
    }
  *pos += dlen;

  printf ("\n");
}

int main (int argc, char *argv[])
{
  FILE *fptr;
  unsigned long pos = 0;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s <filename>\n", argv[0]);
      return 1;
    }

  if (strcmp(argv[1], "-") == 0)
    fptr = stdin;
  else if ((fptr = fopen (argv[1], "rb")) == NULL)
    {
      fprintf (stderr, "Couldn't open file %s\n", argv[1]);
      return 1;
    }

  while (1)
    {
      int channel, length, i, ch;

      if ((ch = getc (fptr)) != '$')
	{
	  if (ch == EOF)
	    return 0;

	  fprintf (stderr, "Couldn't find VLink Start of Packet mark\n");
	  return 1;
	}
      pos++;

      printf ("\nVLink Packet start found\n");

      channel = getc (fptr);
      printf ("Channel: %d\n", channel);
      pos++;

      length = (getc (fptr) << 8) | getc (fptr);
      printf ("Packet Length: %d\n", length);
      pos += 2;

      if (channel == 1 && length >= 8)
	parse_scp (fptr, length, &pos);
      else
	{
	  FILE *f2;

	  if ((f2 = fopen ("lastdata", "wb")) == NULL)
	    for (i = 0; i < length; i++)
	      getc (fptr);
	  else
	    for (i = 0; i < length; i++)
	      putc (getc (fptr), f2);

	  pos += length;

	  fclose (f2);
	}
    }

  fclose (fptr);
}
