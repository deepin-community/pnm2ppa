/***************************************************************************
    pnm2ppa.c  -  program to print PNM input streams to PPA Deskjets
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <getopt.h>
#include <sys/param.h>

#define __PNM2PPA_C__

#include "ppa_syslog.h"
#include "global.h"
#include "image.h"
#include "debug.h"
#include "defaults.h"
#include "ppa.h"
#include "pnm2ppa.h"
#include "gamma.h"
#include "lang.h"


double  Gamma_R, Gamma_G, Gamma_B;
int adjust_coloffsx, adjust_coloffsy;

int
print_pnm (FILE * fptr)
{
    char line[1024];
    image_t image;
    int numpages = 0, pagenum ;
    int res;
    int image_width;

    snprintf(syslog_message,message_size,"%s", 
	     gMessages[LOG_START]);
    wrap_syslog (LOG_INFO,"%s",syslog_message); 

    ppa_init_job (&printer);

    while (initImage (&image, fptr))
    {
      pagenum = numpages + 1;
	DPRINTF ("Initializing page %d...\n ", pagenum);
	ppa_init_page (&printer);
	DPRINTF ("Done!\n");
	DPRINTF ("Loading page %d... \n", pagenum);
	ppa_load_page (&printer);
	DPRINTF ("Done!\n");
	image_width = image.width * 3 * sizeof (int) ;
	DPRINTF ("image.width * 3 * sizeof(int)) == %d\n", 
		 image_width );

	gErrVec = malloc (image_width );

	if (!gErrVec)
	  {
	    snprintf(syslog_message,message_size,"print_pnm(): %s",
		     gMessages[E_BADMALLOC]);
	    wrap_syslog (LOG_CRIT, "%s",syslog_message); 
	    exit (1);
	  }
	memset (gErrVec, 0x00, image_width );

	gErrVec_bw = malloc (image_width );

	if (!gErrVec_bw)
	{
	    snprintf(syslog_message,message_size, "print_pnm(): %s", 
		     gMessages[E_BADMALLOC]);
	    wrap_syslog (LOG_CRIT, "%s",syslog_message); 
	    exit (1);
	}
	memset (gErrVec_bw, 0x00, image_width );

	snprintf(syslog_message,message_size, "%s %d (%s)\n", 
		 gMessages[LOG_PAGE], pagenum, gFormat);
	wrap_syslog (LOG_INFO,  "%s",syslog_message); 


	if ((res = ppa_print_page (&printer, &image)) != 0)
	{
	    DPRINTF ("ppa_print_page returned %d!!\n", res);
	    free (gErrVec);
	    return res;
	}
	free (gErrVec);
	free (gErrVec_bw);
	snprintf(syslog_message,message_size, "%s %d\n", 
		 gMessages[LOG_FINISH], pagenum );
	wrap_syslog (LOG_INFO,  "%s",syslog_message); 

	DPRINTF ("Ejecting page %d... \n", pagenum);
	ppa_eject_page (&printer);
	DPRINTF ("Ejected!\n");

	/* eat any remaining whitespace */
	if (image.version == P1_PBM)
	    fgets (line, 1024, fptr);

	numpages++;
	if (gGammaMode)
	    break;
    }

    if (numpages == 0)
    {
      snprintf(syslog_message,message_size,"%s" , 
	       gMessages[LOG_NOPAGES]);
      wrap_syslog (LOG_INFO,  "%s",syslog_message); 
      return 1;
    }

    ppa_end_print (&printer);

    snprintf(syslog_message,message_size,"%s" , 
	     gMessages[LOG_SUCCESS]);
    wrap_syslog (LOG_INFO,  "%s",syslog_message); 

    fclose (image.fptr);
    fclose (printer.fptr);

    return 0;
}

/* safe transcription of paths specified in user input */
char *
readPath(char *input )
{ 	 
  char File_path[MAXPATHLEN];
  char *path = NULL;
  /* security audit ajv 20001007;  be very careful with input strings!*/
  if (strlen(input) < MAXPATHLEN )
    {
      if ( strncpy (File_path, input, MAXPATHLEN ))
	{
	  /*
	    a user-specifed file *File_path may later be opened;
	    should any extra validation of this be performed now ?
	  */
	  /* remember to free(path) after use! */
	  path = strdup(File_path);
	  return path;
	}
    }
  else
    {
      snprintf(syslog_message,message_size,"%s %d\n",
	       gMessages[E_BAD_PATH], MAXPATHLEN);
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
      abort();
    }
  return path;
}

/* this should be called when you need to set up the printer
 * object for a particular printer.
 */

void
set_printer_specific_defaults (void)
{
    if (printer.version != NOPRINTER && printer.version != HP7X0 &&
	printer.version != HP820 && printer.version != HP1000)
    {
      snprintf(syslog_message,message_size, "set_printer_defaults(): %s",
	       gMessages[E_PPA_UNKNOWN]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
      
      exit (1);
    }

    printer.ColBwOffsX = gPrinterDefaults[printer.version].ColBwOffsX;
    printer.ColBwOffsY = gPrinterDefaults[printer.version].ColBwOffsY;

    printer.x_offset = gPrinterDefaults[printer.version].x_offset;
    printer.y_offset = gPrinterDefaults[printer.version].y_offset;

    printer.r2l_col_offset = gPrinterDefaults[printer.version].r2l_col_offset;
    printer.r2l_bw_offset = gPrinterDefaults[printer.version].r2l_bw_offset;

    printer.top_margin = gPrinterDefaults[printer.version].top_margin;
    printer.bottom_margin = gPrinterDefaults[printer.version].bottom_margin;

    printer.left_margin = gPrinterDefaults[printer.version].left_margin;
    printer.right_margin = gPrinterDefaults[printer.version].right_margin;

    printer.marg_diff = gPrinterDefaults[printer.version].marg_diff;
    printer.bufsize = gPrinterDefaults[printer.version].bufsize;
    printer.min_pagewidth = gPrinterDefaults[printer.version].min_pagewidth;
    printer.max_pagewidth = gPrinterDefaults[printer.version].max_pagewidth;
    printer.min_pageheight = gPrinterDefaults[printer.version].min_pageheight;
    printer.max_pageheight = gPrinterDefaults[printer.version].max_pageheight;}

void
show_usage (char *prog)
{
    printf ("usage: %s [ options ] [-i <infile> ] [ -o <outfile> ]] \n\n",
	    prog);
    printf
	("  Prints a pnm- or pnmraw-format <infile> to HP720/820/1000-format <outfile>.\n\n");
    printf ("    -b <margin>     bottom margin in 1\"/600 (default: 150 = 0.25\")\n");
    printf ("    --bi            forces bidirectional print head sweeps\n");
    printf ("    -B <n>          density of black ink (n = 0,1,2,3 or 4)\n");
    printf ("    --bw            forces black/white output\n");
    printf ("    -d              dumps the configuration to stdout\n");
    printf ("    --eco           Econofast mode (uses less ink)\n");
    printf ("    --dpi300        Treat input resolution as 300dpi instead of 600dpi\n");
    printf ("    -f <cfgfile>    read printer settings from configuration file <cfgfile>\n");
    printf ("    -F <gammaFile>  read color correction table from file <gammaFile>\n");
    printf ("    --fd            use fast dithering mode\n");
    printf ("    -g              print color calibration page. See CALIBRATION.txt\n");
    printf ("    -h,   --help    displays this help text.\n");

    printf ("    -i <infile>     input from file <infile>, \"-i -\" means stdin\n");
    printf ("    -l <margin>     left margin in 1\"/600   (default: 150 = 0.25\")\n");
    printf ("    --noGamma       suppresses any color correction\n");
    printf ("    -o <outfile>    output to file <outfile>, \"-o -\" means stdout\n");
    printf ("    -p              disable black ink cartridge\n");
    printf ("    -r <margin>     right margin in 1\"/600  (default: 150 = 0.25\")\n");
    printf ("    -s <paper>      default papersize (a4, legal, letter = us (default))\n");
    printf ("    -t <margin>     top margin in 1\"/600  (default: 150 = 0.25\")\n");
    printf ("    --uni           forces unidirectional print head sweeps\n");
    printf ("    -v <model>      set the printer model (710, 712, 720, 722, 820, or 1000)\n");
    printf ("    --verbose       outputs syslog output to stderr as well\n");
    printf ("    --version       Display the version of %s to the syslog\n",
	    prog);
    printf ("    -x <offset>     vertical offset adjustment in 1\"/600\n");
    printf ("    -y <offset>     horizontal offset adjustment in 1\"/600\n\n");

    printf
	("  The -v option resets the margins and offset adjustments  to the \n");
    printf
	("  default values for that printer model.  <infile> and <outfile> default\n");
    printf
	("  to stdin and stdout.  '-' is a synonym for stdin and stdout.\n\n");
    printf
	("  Configuration files specified with the '-f' parameter have the following\n  format:\n\n");
    printf ("    # Comment\n");
    printf ("    <key1> <value1>\n");
    printf ("    <key2> <value2>\n");
    printf ("    [etc.]\n\n");
    printf
	("  Valid keys include 'version', 'xoffset', 'yoffset', 'topmargin', 'leftmargin',\n");
    printf
      ("  'rightmargin', 'bottommargin', 'blackness', 'papersize', or any non-null\n");
    printf
      ("  truncated version of these words.  Valid values are the same as with \n");
    printf
      ("  the corresponding command-line parameters.\n");
    printf
      ("  Other keys that can be only set in the configuration file include:\n");
    printf
      ("     'silent', 'verbose', 'log_info',\n");
    printf 
      ("     'unimode', 'blackshear', 'colorshear',\n"); 
    printf
      ("     'RedGammaIdx','BlueGammaIdx', 'GreenGammaIdx',\n");
    printf
      ("     'GammaR', 'GammaG', 'GammaB',\n");
    printf
      ("     'ColOffsX', 'ColOffsY',\n");
    printf
      ("     'black_ink', 'color_ink', 'cyan_ink', 'magenta_ink', 'yellow_ink',\n");
    printf
      ("     'compression';\n");
    printf
      ("  see documentation in the sample configuration file for their usage.\n\n"); 
    printf
      ("  The order in which parameters are specified is important:\n");
    printf
      ("  the file %s, if it exists, is processed as a configuration\n", defaultcfgfile);
    printf 
      ("  file before any command-line parameters are processed.\n");
    printf 
      ("  If an alternate configuration file is specified on the command line\n");
    printf
      ("  with the '-f' parameter, its contents are processed at that point.\n");
    printf
      ("  If a parameter is specified more than once, only its last value is used.\n\n");
}

void
parm_version (char *arg)
{
    


    if (!strcasecmp (arg, "hp720") || !strcmp (arg, "720") ||
	!strcasecmp (arg, "hp722") || !strcmp (arg, "722") ||
	!strcasecmp (arg, "hp710") || !strcmp (arg, "710") ||
	!strcasecmp (arg, "hp712") || !strcmp (arg, "712"))
	printer.version = HP7X0;
    else if (!strcasecmp (arg, "hp820") || !strcmp (arg, "820"))
	printer.version = HP820;
    else if (!strcasecmp (arg, "hp1000") || !strcmp (arg, "1000"))
	printer.version = HP1000;
    else
    {
      snprintf(syslog_message,message_size,"parm_version(): %s", 
	       gMessages[E_PPA_UNKNOWN]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

      exit (1);
    }
    set_printer_specific_defaults ();
}

void
parm_iversion (int arg)
{
    switch (arg)
    {      
    case 0:
	printer.version = NOPRINTER;
	break;
    case 720:
    case 722:
    case 710:
    case 712:
	printer.version = HP7X0;
	break;
    case 820:
	printer.version = HP820;
	break;
    case 1000:
	printer.version = HP1000;
	break;
    default:
	{
	  snprintf(syslog_message,message_size,"parm_iversion():  %s",
		   gMessages[E_PPA_UNKNOWN]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	  exit (1);
	}
    }
    set_printer_specific_defaults ();
}

void
dump_config (void)
{
    printf ("pnm2ppa version " VERSION "\n\n");

    printf ("Printer:   ");
    switch (printer.version)
    {
    case HP7X0:
	printf ("HP 710c, 712c, 720c or 722c\n\n");
	break;

    case HP820:
	printf ("HP820\n\n");
	break;

    case HP1000:
	printf ("HP1000\n\n");
	break;

    case NOPRINTER:
    default:
	printf ("unknown printer\n");
	exit (1);
    }

    printf ("Color Head Offset Adjustments:\n  ColOffsX:  %d\n  ColOffsY:  %d\n\n",
	    adjust_coloffsx, adjust_coloffsy);

    printf ("Offsets:\n  xoffset:  %d\n  yoffset:  %d\n\n",
	    printer.x_offset, printer.y_offset);

    printf ("Shearing corrections for bidirectional printing (right_to_left):\n    blackshear:  %d\n    colorshear:  %d\n\n",
	    printer.r2l_bw_offset, printer.r2l_col_offset);

    printf ("Margins:\n\ttop:\t%d\n\tbottom:\t%d\n",
	    printer.top_margin, printer.bottom_margin);
    printf ("\tleft:\t%d\n\tright:\t%d\n\n",
	    printer.left_margin, printer.right_margin);
    printf ("Gamma values:\n\tRed:\t%f\n\tGreen:\t%f\n\tBlue:\t%f\n",
	    Gamma_R, Gamma_G, Gamma_B );
    exit (0);
}

/* only let gSilent be set to switch off syslog from $SYSCONFDIR/etc/pnm2ppa.conf, 
   and  not from
   subsequent readings of config files 
*/
BOOLEAN system_config_file ;

void
read_config_file (char *fname)
{
  FILE *cfgfile = fopen (fname, "r");
  char line[1024], key[14], buf[10];
  int len, value, count, lineno = 1;
  
  if (!cfgfile)
    {
      snprintf(syslog_message,message_size,"read_config_file(): %s", 
	       gMessages[E_BADCONFIGFILE] );
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

      exit (1);
    }

  while (fgets (line, 1024, cfgfile))
    {
      if (strchr (line, '#'))
	*strchr (line, '#') = 0;
      count = sscanf (line, "%13s%9s", key, buf);
      if (count == 2)
	{
	  DPRINTF("config file %s: keyword %s value %s\n",fname,key,buf);
	}
      switch (count)
	{
	case 2:
	  value = atoi (buf);
	  len = strlen (key);
	  if (!strncmp (key, "version", len))
	    parm_iversion (value);
	  else if (!strncmp (key, "xoffset", len))
	    printer.x_offset = value;
	  else if (!strncmp (key, "yoffset", len))
	    printer.y_offset = value;
	  else if (!strncmp (key, "blackshear", len))
	    printer.r2l_bw_offset = value;
	  else if (!strncmp (key, "colorshear", len))
	    printer.r2l_col_offset = value;
	  else if (!strncmp (key, "dump", len))
	    dump_config ();
	  else if (!strncmp (key, "topmargin", len))
	    printer.top_margin = value;
	  else if (!strncmp (key, "leftmargin", len))
	    printer.left_margin = value;
	  else if (!strncmp (key, "rightmargin", len))
	    printer.right_margin = value;
	  else if (!strncmp (key, "bottommargin", len))
	    printer.bottom_margin = value;
	  else if (!strncmp (key, "GammaR", len))
	    Gamma_R = atof(buf);
	  else if (!strncmp (key, "GammaG", len))
	    Gamma_G = atof(buf);
	  else if (!strncmp (key, "GammaB", len))
	    Gamma_B = atof(buf);
	  else if (!strncmp (key, "RedGammaIdx", len))
	    Gamma_R = GammaValue(value);
	  else if (!strncmp (key, "GreenGammaIdx", len))
	    Gamma_G = GammaValue(value);
	  else if (!strncmp (key, "BlueGammaIdx", len))
	    Gamma_B = GammaValue(value);
	  else if (!strncmp (key, "ColOffsX", len))
	    adjust_coloffsx = value;
	  else if (!strncmp (key, "ColOffsY", len))
	    adjust_coloffsy = value;
	  else if (!strncmp (key, "blackness", len))
	    gBlackness = value;
	  else if (!strncmp (key, "unimode", len))
	    {
	      if ( value )
		gUnimode = true ;
	      else
		gUnimode = false ;
	    }
	  else if (!strncmp (key, "black_ink", len))
	    {
	      if ( value )
		gBlack = true ;
	      else
		gBlack = false ;
	    }
	  else if (!strncmp (key, "cyan_ink", len))
	    {
	      if ( value )
		gCyan = true ;
	      else
		gCyan = false ;
	    }
	  else if (!strncmp (key, "magenta_ink", len))
	    {
	      if ( value )
		gMagenta = true ;
	      else
		gMagenta = false ;
	    }
	  else if (!strncmp (key, "yellow_ink", len))
	    {
	      if ( value )
		gYellow = true ;
	      else
		gYellow = false ;
	    }
	  else if (!strncmp (key, "color_ink", len))
	    {
	      if ( value )
		{		
		  gCyan = true ;
		  gMagenta = true ;
		  gYellow = true ;
		}
	      else
		{		
		  gCyan = false ;
		  gMagenta = false ;
		  gYellow = false ;
		}
	    }
	  else if (!strncmp (key, "silent", len))
	    {
	      if (system_config_file)
		{
		  gSilent = true;
		  wrap_closelog();
		}
	    }
	  else if (!strncmp (key, "log_info", len))
	    {
	      if (system_config_file) {
		if ( value )
		  gLogInfo = true ;
		else
		  gLogInfo = false ;
	      }
	    }
	  else if (!strncmp (key, "verbose", len))
	    {
	      gVerbose  = true;
	      if (!(gSilent))
		{
		 wrap_closelog ();
		 wrap_openlog ("pnm2ppa", 1);
		 }
	      snprintf(syslog_message,message_size, "pnm2ppa: %s", 
		       gMessages[LOG_VERBOSE]);
	      wrap_syslog(LOG_INFO,"%s",syslog_message); 
	      
	    }
	  else if (!strncmp (key, "papersize", len))
	    {
	      if (!strncmp (buf, "us", 2) || !strncmp (buf, "letter", 6))
		{
		  gWidth = LETTERWIDTH;
		  gHeight = LETTERHEIGHT;
		}
	      else if (!strncmp (buf, "legal", 5))
		{
		  gWidth = LEGALWIDTH;
		  gHeight = LEGALHEIGHT;
		}
	      else if (!strncmp (buf, "a4", 2))
		{
		  gWidth = A4WIDTH;
		  gHeight = A4HEIGHT;
		}
	      else
		{
		  snprintf(syslog_message,message_size,"read_config_file(): %s",
			   gMessages[E_BAD_PAPER] );
		  wrap_syslog (LOG_CRIT,"%s",syslog_message); 
		  
		  exit (1);
		}
	    }
	  else if (!strncmp (key, "compression", len))
	    {
	      if ( value )
		gCompress = true ;
	      else
		gCompress = false ;
	    }
	  else
	    {
	      snprintf(syslog_message,message_size,"read_config_file(): %s (%s %d)\n",
		       gMessages[E_BADPARM] , gMessages[LINE],lineno );
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	      show_usage ("pnm2ppa");
	      exit (1);
	    }
	case EOF:
	case 0:
	  break;
	default:
	  snprintf(syslog_message,message_size, "read_config_file(): %s %s %d\n",
		   gMessages[E_PARSE_CONFIG], gMessages[LINE], lineno);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	  exit (1);
	}
      lineno++;
    }
  
  if (feof (cfgfile))
    {
      fclose (cfgfile);
      return;
    }
  snprintf(syslog_message,message_size,"read_config_file(): %s\n",
	   gMessages[E_PARSE_CONFIG]);
  wrap_syslog (LOG_CRIT,"%s",syslog_message); 

  exit (1);
}

int
main (int argc, char *argv[])
{
  int i, opt_index;
  FILE *in = NULL, *out = NULL, *gf = NULL;
  struct stat tmpstat;
  int c;
  BOOLEAN get_Gamma = true;

  char gammaFile[MAXPATHLEN];
  char pnm_inputFile[MAXPATHLEN];
  char ppa_outputFile[MAXPATHLEN];
  BOOLEAN use_stdin = false;
  BOOLEAN use_stdout = false;
  BOOLEAN got_Input = false;
  BOOLEAN got_Output = false;
  BOOLEAN got_Gamma = false;
  gSilent = false;
  gVerbose = false;
  
  system_config_file = true;

#ifdef __NO_SYSLOG__
  gSilent = true;
#endif

  strncpy(gammaFile,defaultgammafile,MAXPATHLEN);
  
  /* activate syslog */
  if (!(gSilent))
    {
#ifdef DEBUG
      wrap_openlog( "pnm2ppa", 1);
#else
      wrap_openlog( "pnm2ppa", 0);
#endif
    }

  if (argc < 2)
    {
      show_usage (*argv);
      exit (0);
    }
  
  /* setup the printer object */
  
  printer.version = HP7X0;
  set_printer_specific_defaults ();
  
  /* initialize global variables with default values */

  gEcoMode = false;
  gWidth = LETTERWIDTH;
  gHeight = LETTERHEIGHT;
  gBlackness = 2;
  gPixmapMode = false;
  gGammaMode = false;
  gColorMode = true;
  gFastMode = false;
  gUnimode = false;
  g300 = false;
  gCompress = true;
  gLogInfo = false;       /* no  LOG_INFO syslog messages by default */ 
  gBlack = true;
  gCyan = true;
  gMagenta = true;
  gYellow = true;
  Gamma_R = 1.0;
  Gamma_G = 1.0;
  Gamma_B = 1.0;
  
  adjust_coloffsx = 0;
  adjust_coloffsy = 0;

  
  if (!stat (defaultcfgfile, &tmpstat))
    read_config_file (defaultcfgfile);

  system_config_file = false;

  /* read through the optional arguments before tring to find filenames */
  
  while ((c = getopt_long (argc, argv, "F:B:b:df:ghi:l:o:pr:s:t:v:x:y:",
			   long_opts, &opt_index)) != EOF)
    {
      switch (c)
	{
	case 0:
	  /* long options */
	  switch (opt_index)
	    {
	    case 0:
	      /* --bw               Black & White */ 
	      gColorMode = false;
#if 0
	      gEcoMode = false;   /*this is not needed here (?) duncan*/
#endif
	      gMaxPass = 1;
	      break;
	      
	    case 1:
	      /* --fd               Fast Dither*/
	      gFastMode = true;
	      gMaxPass = 1;
	      break;
	      
	    case 2:
	      /* --version (output to stderr, not syslog) */
	      fprintf(stderr,"%s  %s  %s  %s %s\n",
		      *argv, gMessages[LOG_VERSION], 
		       VERSION, __DATE__, __TIME__);
	      exit (0);
	      break;
	      
	    case 3:
	      /* --help */
	      show_usage (*argv);
	      return 0;
	      break;

	    case 4:
	      /* --verbose */
	      gVerbose = true;
	      if (!(gSilent))
		{
		  wrap_closelog ();
		  wrap_openlog ("pnm2ppa", 1);
		}
	      break;
	      
	    case 5:
	      /* --eco */
	      gEcoMode = true;
	      break;
	      
	    case 6:
	      /* --noGamma */
	      get_Gamma = false;
	      break;
	      
	    case 7:
	      /* --uni */
	      gUnimode = true;
	      break;
	      
	    case 8:
	      /* --bi */
	      gUnimode=false;
	      break;

	    case 9:
	      /* --dpi300 */
	      g300=true;
	      break;
	      
	    default:
	      snprintf(syslog_message,message_size,"main (): %s",
		       gMessages[E_UNKNOWN_ARG]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	      DPRINTF(" unknown longopt switch case should not happen");
	      return 1;
	      break;
	    }
	  break;
	  
	case 'b':
	  printer.bottom_margin = atoi (optarg);
	  break;
	  
	case 'B':
	  gBlackness = atoi(optarg);
	  break;
	  
	case 'd':
	  dump_config ();
	  break;
	  
	case 'f':
	  {
	    char *configFile=NULL;
	    configFile = readPath (optarg);
	    if (configFile) {
	      read_config_file (configFile);
	      free(configFile);
	    }  
	  }
	  break;
	  
	case 'F':
	  {
	    char *path=NULL;
	    path = readPath (optarg);
	    if (path) {
	      strncpy(gammaFile,path,MAXPATHLEN);
	      free(path);
	    }
	  }
	  break;
	  
	case 'g':
	  gGammaMode = true;
	  break;
	  
	case 'h':
	  show_usage (*argv);
	  return 0;
	  break;
	  
	case 'i':
	  if (strncmp (optarg, "-", MAXPATHLEN) == 0)
	    {
	      use_stdin = true;
	      got_Input = true;
	    }
	  else
	    {
	      char *path=NULL;
	      path = readPath (optarg);
	      if (path)
		{
		  strncpy(pnm_inputFile,path,MAXPATHLEN);
		  free(path);
		  use_stdin = false;
		  got_Input = true;
		}
	    }
	  break;
	  
	case 'l':
	  printer.left_margin = atoi (optarg);
	  break;
	  
	case 'o':
	  if (strncmp (optarg, "-", MAXPATHLEN) == 0)
	    {
	      use_stdout = true;
	      got_Output = true;
	    }
	  else
	    {
	      char *path=NULL;
	      path = readPath(optarg);
	      if (path)
		{
		  strncpy(ppa_outputFile,path,MAXPATHLEN);
		  free(path);
		  use_stdout = false ;
		  got_Output = true;
		}
	    }
	  break;
	  
	case 'p':
	  gPixmapMode = true;
	  break;
	  
	case 'r':
	  printer.right_margin = atoi (optarg);
	  break;
	  
	case 's':
	  /*  paper size stuffer (only used in  -g  mode ) */
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
	      snprintf(syslog_message,message_size,"main(): %s ",
		       gMessages[E_BAD_PAPER] );
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	      return 1;
	    }

	    break;

	case 't':
	    printer.top_margin = atoi (optarg);
	    break;

	case 'v':
	    parm_version (optarg);
	    break;

	case 'x':
	    printer.x_offset = atoi (optarg);
	    break;

	case 'y':
	  printer.y_offset = atoi (optarg);
	  break;

	default:
	  snprintf(syslog_message,message_size,"main(): %s",
		   gMessages[E_UNKNOWN_ARG]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	    return 1;
	    break;
	}
    }

  /* open input stream, if already specified */
  if (got_Input)
    {
      if (use_stdin)
	in = stdin;
      else
	{
	  if ((in = fopen (pnm_inputFile, "rb")) == NULL)
	    {
	      snprintf(syslog_message,message_size,"main(): %s",
		       gMessages[E_BAD_INPUT]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	      return 1;
	    }
	}
    }

  /* open output stream, if already specified */
  if (got_Output)
    {
      if (use_stdout)
	out = stdout;
      else
	{
	  if ((out = fopen (ppa_outputFile, "wb")) == NULL)
	    {
	      snprintf(syslog_message,message_size,"main(): %s",
		       gMessages[E_BAD_OUTPUT] );
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	      return 1;
	    }
	}
    }      


  
  
  /* move on to the list argument*/
  
  argc -= optind;
  argv += optind;

    /* if the user hasn't yet specified -i or -o, we see if there are
       any remaining arguments. Only allows both arguments to work here.
       This is a legacy option style used by pbm2ppa
    */
  
  if (argc == 2 && !in && !out)
    {
      if (strncmp (argv[1], "-", MAXPATHLEN) == 0)
	{
	  in = stdin;
	}
      else
	{
	  if ((in = fopen (argv[0], "rb")) == NULL)
	    {
	      snprintf(syslog_message,message_size,"main():  %s",
		      gMessages[E_BAD_INPUT]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	      return 1;
	    }
	}
      if (strncmp (argv[1], "-", MAXPATHLEN) == 0)
	{
	  out = stdout;
	}
      else
	{
	  if ((out = fopen (argv[1], "wb")) == NULL)
	    {
	      snprintf(syslog_message,message_size,"main():  %s",
		       gMessages[E_BAD_OUTPUT]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	      
	      return 1;
		       }
	}
    }
  
  /* if no input is specified, we terminate */
  /* if the user doesn't specify  output file, we make like a pipe*/
  
  if (in == NULL)
    exit(0);    
  if (out == NULL)
    out = stdout;
  
  printer.fptr = out;
  
    
  /* set minimum and maximum paper sizes for the printer
     (the DeskJet 1000C  has different limits from other models )
  */
  
  gminWidth  = printer.min_pagewidth ;
  gmaxWidth  = printer.max_pagewidth ;
  gminHeight = printer.min_pageheight ;
  gmaxHeight = printer.max_pageheight ;
	
  
  /* set gMaxPass 
     (number of sweeps for Floyd-Steenberg dithered color printing)
  */

     gMaxPass = 1;
     if ( gColorMode && !(gEcoMode) )
       gMaxPass = 4;
     
     
     /* adjust color/bw offsets (alignment corrections) */
     printer.ColBwOffsY += adjust_coloffsy;
     printer.ColBwOffsX += adjust_coloffsx;


     /* initialize color correction curve 
	(used in Floyd-Steenberg dithered color) 
     */
  
  
  /* initialize for no color correction */

    for (i = 0; i < 256; i++)
    {
	gEnh_curve_r[i] = i;
	gEnh_curve_g[i] = i;
	gEnh_curve_b[i] = i;
    }


    /*  if color correction is used */

    if (get_Gamma)
    {
      gf = fopen (gammaFile, "rb");
	if (gf == NULL)
	  {
	    DPRINTF ("Can't open gamma correction curve file %s!\n",	
	     gammaFile);
	  }
	else
	{
	  int num_read=0;
	  got_Gamma = true ;
	  /* read the gamma correction curve from gammaFile */
	  DPRINTF("main(): reading color correction curve from %s\n",gammaFile);

	  num_read = fread (gEnh_curve_r, sizeof (int), 256, gf) ;
	  DPRINTF(" num_read= %d\n",num_read);
	  if ( num_read != 256 )
	    got_Gamma = false ;

	  if (got_Gamma)
	    {
	      num_read = fread (gEnh_curve_g, sizeof (int), 256, gf);
	      DPRINTF(" num_read= %d\n",num_read); 
	      if ( num_read != 256 )
		got_Gamma = false ;
	    }
	  if (got_Gamma)
	    {
	      num_read =  fread (gEnh_curve_b, sizeof (int), 256, gf) ;
	      DPRINTF(" num_read= %d\n",num_read);
	      if ( num_read != 256 )
		got_Gamma = false ;
	    }
	  fclose (gf);
	  if (!(got_Gamma))
	    {
	      DPRINTF ("main(): Gamma curve data from %s is truncated.\n",
		       gammaFile );
	      snprintf(syslog_message,message_size,"main():  %s",
		      gMessages[E_BAD_GAMMAFILE]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	      return 1;
	    }
	}

	if ( !(got_Gamma) )
	  {
	    /* use the empirical color correction curve parametrized by 
	       Gamma_R ,Gamma_G, Gamma_B */
	    DPRINTF("main(): using empirical color correction curve\n");
	    
	    Make_GammaCurve ( Gamma_R, Gamma_G, Gamma_B);

	  }
	
    }

    /* if printer version is set to NOPRINTER in pnm2ppa.conf  */
    if (printer.version == NOPRINTER) {
      snprintf(syslog_message,message_size,"pnm2ppa.conf: version 0; %s", 
	       gMessages[E_PPA_UNKNOWN]);
      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

      exit (1);
    }

    /* now do the work! */

    return print_pnm (in);
}

