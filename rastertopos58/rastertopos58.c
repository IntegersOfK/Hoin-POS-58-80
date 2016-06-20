/*
 * "$Id: rastertozj.c,v 1.1 2001/08/30 19:37:33 mike Exp $"
 *
 *   Zjiang printer filter for the Common UNIX
 *   Printing System (CUPS).
 *
 *   Copyright 2007-2011 by Zjiang .
 *
 * Contents:
 *
 *   Setup()        - Prepare the printer for printing.
 *   StartPage()    - Start a page of graphics.
 *   EndPage()      - Finish a page of graphics.
 *   Shutdown()     - Shutdown the printer.
 *   CancelJob()    - Cancel the current job...
 *   main()         - Main entry and processing of driver.
 */
/*
 * Include necessary headers...
 */
#include <cups/cups.h>
#include <cups/raster.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>

   
#define ALLIGN_TO_32BIT(X)  ((X+31)&(~31))
#define ALLIGN_TO_16BITS(X)  ((X+15)&(~15))
#define ALLIGN_TO_8BITS(X)  ((X+7)&(~7))
#define HEIGHT_PIXEL	24

#define EJECT_CASH_DRAWER1_BEFORE_PRINT	1
#define EJECT_CASH_DRAWER2_BEFORE_PRINT	2
#define EJECT_CASH_DRAWER12_BEFORE_PRINT	3
#define EJECT_CASH_DRAWER1_AFTER_PRINT	4
#define EJECT_CASH_DRAWER2_AFTER_PRINT	5
#define EJECT_CASH_DRAWER12_AFTER_PRINT	6

#define BEEP_AFTER_PAGE	1
#define BEEP_BEFORE_PAGE	2
#define BEEP_AFTER_DOC	3
#define BEEP_BEFORE_DOC	4

#define DO_NOT_PRINT_LOGO	0

#define PRINT_LOGO1	1
#define PRINT_LOGO2	2
#define PRINT_LOGO3	3
#define PRINT_LOGO4	4
#define PRINT_LOGO5	5
#define PRINT_LOGO6	6
#define PRINT_LOGO7	7
#define PRINT_LOGO8	8
/*
 * Globals...
 */

int		Page;			/* Current page number */

int     cashDrawerSetting;
int     blankSpaceSetting;
int     feedDistSetting;
int     beeperSetting;
int     logoSetting;

/*
 * Prototypes...
 */
void	Setup(void);
void	StartPage(ppd_file_t *ppd, cups_page_header2_t *header);
void	EndPage(void);
void	Shutdown(void);
void	CancelJob(int sig);

static int getPaperWidth();
static void ejectCashDrawer(int no);
static void beep();
static void printLogo(int logoNo);

/*
 * 'Setup()' - Prepare the printer for printing.
 */
void
Setup(void)
{

	if(cashDrawerSetting == EJECT_CASH_DRAWER1_BEFORE_PRINT){
		ejectCashDrawer(1);
	}else if(cashDrawerSetting == EJECT_CASH_DRAWER2_BEFORE_PRINT){
		ejectCashDrawer(2);
	}else if(cashDrawerSetting == EJECT_CASH_DRAWER12_BEFORE_PRINT){
		ejectCashDrawer(3);
        }

	if(beeperSetting == BEEP_BEFORE_DOC){
		beep();
	}

	/*
	 * Send a reset sequence.
	 */
	putchar(0x1b);
	putchar(0x40);
}

/*
 * 'Shutdown()' - Shutdown the printer.
 */
void
Shutdown(void)
{

	if(beeperSetting == BEEP_AFTER_DOC){
		beep();
	}

	if(cashDrawerSetting == EJECT_CASH_DRAWER1_AFTER_PRINT){
		ejectCashDrawer(1);
	}else if(cashDrawerSetting == EJECT_CASH_DRAWER2_AFTER_PRINT){
		ejectCashDrawer(2);
	}else if(cashDrawerSetting == EJECT_CASH_DRAWER12_AFTER_PRINT){
		ejectCashDrawer(3);
	}

	/*
	 * Send a reset sequence.
	 */
	putchar(0x1b);
	putchar(0x40);
}

/*
 * 'StartPage()' - Start a page of graphics.
 */

void
StartPage(ppd_file_t         *ppd,	/* I - PPD file */
          cups_page_header2_t *header)	/* I - Page header */
{
	int	plane;				/* Looping var */
#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
	struct sigaction action;		/* Actions for POSIX signals */
#endif /* HAVE_SIGACTION && !HAVE_SIGSET */


	/*
	 * Register a signal handler to eject the current page if the
	 * job is cancelled.
	 */

#ifdef HAVE_SIGSET /* Use System V signals over POSIX to avoid bugs */
	sigset(SIGTERM, CancelJob);
#elif defined(HAVE_SIGACTION)
	memset(&action, 0, sizeof(action));

	sigemptyset(&action.sa_mask);
	action.sa_handler = CancelJob;
	sigaction(SIGTERM, &action, NULL);
#else
	signal(SIGTERM, CancelJob);
#endif /* HAVE_SIGSET */

 /*
  * Setup printer/job attributes...
  */
	if(beeperSetting == BEEP_BEFORE_PAGE){
		beep();
	}

	if(logoSetting > DO_NOT_PRINT_LOGO){
		printLogo(logoSetting);
	}
 /*
  * Set graphics mode...
  */


 /*
  * Set size and position of graphics...
  */

 /*
  * Allocate memory for a line of graphics...
  */
}


/*
 * 'EndPage()' - Finish a page of graphics.
 */

void
EndPage(void)
{
#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
  struct sigaction action;	/* Actions for POSIX signals */
#endif /* HAVE_SIGACTION && !HAVE_SIGSET */


 /*
  * Eject the current page...
  */

  fflush(stdout);

 /*
  * Unregister the signal handler...
  */

#ifdef HAVE_SIGSET /* Use System V signals over POSIX to avoid bugs */
  sigset(SIGTERM, SIG_IGN);
#elif defined(HAVE_SIGACTION)
  memset(&action, 0, sizeof(action));

  sigemptyset(&action.sa_mask);
  action.sa_handler = SIG_IGN;
  sigaction(SIGTERM, &action, NULL);
#else
  signal(SIGTERM, SIG_IGN);
#endif /* HAVE_SIGSET */

  //handle feedDist option
  int feedDistArray[15] = {3,6,9,12,15,18,21,24,27,30,33,36,39,42,45};
  int dist = feedDistArray[feedDistSetting];

  for(; dist > 0; dist -= 3)
  {
#define CMD_LEN	8
		unsigned char buf[2048];
		int lineWidthBytes;

		buf[0] = 0x1d;
		buf[1] = 0x76;
		buf[2] = 0x30;
		buf[3] = 00;
		buf[4] = (unsigned char)(lineWidthBytes%256);
		buf[5] = (unsigned char)(lineWidthBytes/256);
		buf[6] = (unsigned char)(HEIGHT_PIXEL%256);
		buf[7] = (unsigned char)(HEIGHT_PIXEL/256);

		lineWidthBytes = ALLIGN_TO_8BITS(getPaperWidth())/8;

		memset(buf + CMD_LEN, 0, HEIGHT_PIXEL * lineWidthBytes);
		fwrite(buf, HEIGHT_PIXEL * lineWidthBytes + CMD_LEN , 1, stdout);
		buf[0] =0x1b;
		buf[1] =0x4a;
		buf[2] =0x15;
		fwrite(buf, 3, 1, stdout);
#undef CMD_LEN
  }
  //handle feedDist option end.

  if(beeperSetting == BEEP_AFTER_PAGE){
	  beep();
  }
 /*
  * Free memory...
  */
}




/*
 * 'CancelJob()' - Cancel the current job...
 */

void
CancelJob(int sig)			/* I - Signal */
{
  int	i;				/* Looping var */


  (void)sig;

 /*
  * Send out lots of NUL bytes to clear out any pending raster data...
  */

  for (i = 0; i < 600; i ++)
    putchar(0);

 /*
  * End the current page and exit...
  */

  EndPage();
  Shutdown();

  exit(0);
}




#define BYTES_PER_LINE 200
/*
 * 'main()' - Main entry and processing of driver.
 */

int 			/* O - Exit status */
main(int  argc,		/* I - Number of command-line arguments */
     char *argv[])	/* I - Command-line arguments */
{
	int			fd;	/* File descriptor */
	cups_raster_t		*ras;	/* Raster stream for printing */
	cups_page_header2_t	header;	/* Page header from file */
	int			y;	/* Current line */
	int			plane;	/* Current color plane */
	ppd_file_t		*ppd;	/* PPD file */
	unsigned char dest [2048];
	int lineWidth;
	int lineWidthBytes;
	int blankheight;

	cups_option_t *options;
	int num_options;
	ppd_choice_t *choice;

	/*
	 * Make sure status messages are not buffered...
	*/
	setbuf(stderr, NULL);

	/*
	 * Check command-line...
	 */
	if (argc < 6 || argc > 7)
	{
		/*
		 * We don't have the correct number of arguments; write an error message
		 * and return.
		 */

		fputs("ERROR: rastertopcl job-id user title copies options [file]\n", stderr);
		return (1);
	}

	/*
	 * Open the page stream...
	 */

	if (argc == 7)
	{
		if ((fd = open(argv[6], O_RDONLY)) == -1)
		{
			perror("ERROR: Unable to open raster file - ");
			sleep(1);
			return (1);
		}
	}
	else
		fd = 0;

	ras = cupsRasterOpen(fd, CUPS_RASTER_READ);
	/*
	 * Initialize the print device...
	 */
	ppd = ppdOpenFile(getenv("PPD"));
	options = NULL;
	num_options = cupsParseOptions(argv[5], 0, &options);
   
	ppdMarkDefaults(ppd);
	cupsMarkOptions(ppd, num_options, options);
	cupsFreeOptions(num_options, options);
	//read user settings for this job.
	choice = ppdFindMarkedChoice(ppd, "CashDrawer");
   if( choice != NULL ){
	    cashDrawerSetting = atoi(choice->choice);
    }
	choice = ppdFindMarkedChoice(ppd, "BlankSpace");
	if( choice != NULL ){
        blankSpaceSetting = atoi(choice->choice);
    }
	choice = ppdFindMarkedChoice(ppd, "FeedDist");
	if( choice != NULL ){
       feedDistSetting = atoi(choice->choice);
    }
	choice = ppdFindMarkedChoice(ppd, "Beeper");
	if( choice != NULL ){
       beeperSetting = atoi(choice->choice);
    }
	choice = ppdFindMarkedChoice(ppd, "NVLogo");
	if( choice != NULL ){
       logoSetting = atoi(choice->choice);
    }
	Setup();
	/*
	 * Process pages as needed...
	 */

	Page = 0;

	while (cupsRasterReadHeader2(ras, &header))
	{
		/*
		 * Write a status message with the page number and number of copies.
		 */

		Page ++;

		fprintf(stderr, "PAGE: %d %d\n", Page, header.NumCopies);

		/*
		 * Start the page...
		 */

		blankheight = 0;
		StartPage(ppd, &header);

		/*
		 * Loop for each line on the page...
		 */

		lineWidth = header.cupsWidth;

		if(lineWidth > getPaperWidth()){
			lineWidth = getPaperWidth();
		}
		lineWidthBytes = ALLIGN_TO_8BITS(lineWidth)/8;

		for (y = 0; y < header.cupsHeight; )
		{
			/*
			 * Let the user know how far we have progressed...
			 */

			if ((y & 127) == 0)
				fprintf(stderr, "INFO: Printing page %d, %d%% complete...\n", Page,
						100 * y / header.cupsHeight);
#define CMD_LEN	8
			dest[0] = 0x1d;
			dest[1] = 0x76;
			dest[2] = 0x30;
			dest[3] = 00;
			dest[4] = (unsigned char)(lineWidthBytes%256);
			dest[5] = (unsigned char)(lineWidthBytes/256);

			int h;
			if(header.cupsHeight - y > HEIGHT_PIXEL){
				//image has been clipped, so we always print the fixed height.
				h = HEIGHT_PIXEL;
			}else{
				h = header.cupsHeight - y;
			}
			y += h;

			dest[6] = (unsigned char)(h%256);
			dest[7] = (unsigned char)(h/256);


			/*
			 * Read h line of graphics...
			 */

			int i;
			for(i = 0; i < h ; i++){
				if (cupsRasterReadPixels(ras, dest + CMD_LEN + i * lineWidthBytes, header.cupsBytesPerLine) < 1)
					break;
			}
			if(i < h){
				break;
			}

			/*
			 * See if the line is blank; if not, write it to the printer...
			 */
			if (dest[CMD_LEN] || memcmp(dest + CMD_LEN, dest + CMD_LEN + 1, h * lineWidthBytes - 1)) {
				//output blank line first.
				unsigned char buf[2048];

				while(blankheight--){
					buf[0] = 0x1d;
					buf[1] = 0x76;
					buf[2] = 0x30;
					buf[3] = 00;
					buf[4] = (unsigned char)(lineWidthBytes%256);
					buf[5] = (unsigned char)(lineWidthBytes/256);
					buf[6] = (unsigned char)(HEIGHT_PIXEL%256);
					buf[7] = (unsigned char)(HEIGHT_PIXEL/256);

					memset(buf + CMD_LEN, 0, HEIGHT_PIXEL * lineWidthBytes);
					fwrite(buf, HEIGHT_PIXEL * lineWidthBytes + CMD_LEN , 1, stdout);
					buf[0] =0x1b;
					buf[1] =0x4a;
					buf[2] =0x15;
					fwrite(buf, 3, 1, stdout);
				}

				fwrite(dest, h * lineWidthBytes + CMD_LEN , 1, stdout);
				buf[0] =0x1b;
				buf[1] =0x4a;
				buf[2] =0x15;
				fwrite(buf, 3, 1, stdout);

				blankheight = 0;
			}else{
				blankheight++;
			}


		}

		if(blankSpaceSetting == 0){
			unsigned char buf[3];

			while(blankheight--){
				dest[0] = 0x1d;
				dest[1] = 0x76;
				dest[2] = 0x30;
				dest[3] = 00;
				dest[4] = (unsigned char)(lineWidthBytes%256);
				dest[5] = (unsigned char)(lineWidthBytes/256);
				dest[6] = (unsigned char)(HEIGHT_PIXEL%256);
				dest[7] = (unsigned char)(HEIGHT_PIXEL/256);

				memset(dest + CMD_LEN, 0, HEIGHT_PIXEL * lineWidthBytes);
				fwrite(dest, HEIGHT_PIXEL * lineWidthBytes + CMD_LEN , 1, stdout);
				buf[0] =0x1b;
				buf[1] =0x4a;
				buf[2] =0x15;
				fwrite(buf, 3, 1, stdout);
			}
		}
		EndPage();
	}

	/*
	 * Shutdown the printer...
	 */

	Shutdown();

	if (ppd)
		ppdClose(ppd);

	/*
	 * Close the raster stream...
	 */

	cupsRasterClose(ras);
	if (fd != 0)
		close(fd);

	/*
	 * If no pages were printed, send an error message...
	 */

	if (Page == 0)
		fputs("ERROR: No pages found!\n", stderr);
	else
		fputs("INFO: Ready to print.\n", stderr);

	return (Page == 0);
#undef CMD_LEN
}

static int getPaperWidth()
{
    return 384;
}

static void ejectCashDrawer(int no)
{
	char buf[10];

	if(no == 1){
		buf[0] = 0x1b;
		buf[1] = 'p';
		buf[2] = 0x00;
		buf[3] = 0x40;
		buf[4] = 0x50;
		fwrite(buf, 5, 1, stdout);
	}else if(no == 2){
		buf[0] = 0x1b;
		buf[1] = 'p';
		buf[2] = 0x01;
		buf[3] = 0x40;
		buf[4] = 0x50;
		fwrite(buf, 5, 1, stdout);
	}else if(no == 3){
		buf[0] = 0x1b;
		buf[1] = 'p';
		buf[2] = 0x00;
		buf[3] = 0x40;
		buf[4] = 0x50;
		buf[5] = 0x1b;
		buf[6] = 'p';
		buf[7] = 0x01;
		buf[8] = 0x40;
		buf[9] = 0x50;
		fwrite(buf, 10, 1, stdout);
	}
}

static void beep()
{
	unsigned char buf[4];

	buf[0] = 0x1b;
	buf[1] = 0x42;
	buf[2] = 0x04;
	buf[3] = 0x01;
	fwrite(buf, 4, 1, stdout);
}

static void printLogo(int logoNo)
{
	unsigned char buf[7];

	buf[0] = 0x1b;
	buf[1] = 'a';
	buf[2] = 0x01;
	buf[3] = 0x1c;
	buf[4] = 'p';
	buf[5] = 0x01;
	buf[6] = 0x00;
	fwrite(buf, 7, 1, stdout);
}

/*
 * End of "$Id: rastertozj.c,v 1.1 2011/01/15 19:37:33 mike Exp $".
 */
