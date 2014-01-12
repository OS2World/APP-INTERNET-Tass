
/*
 *  This is a screen management library borrowed with permission from the
 *  Elm mail system (a great mailer--I highly recommend it!).
 *
 *  I've hacked this library to only provide what Tass needs.
 *
 *  Original copyright follows:
 */

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 2.1 $   $State: Exp $
 *
 * 			Copyright (c) 1986 Dave Taylor
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#if !(defined(AMIGA) || defined (OS2))
#include <curses.h>
#endif
#ifdef OS2
#undef SHORT
#define INCL_NOPM
#define INCL_SUB
#define INCL_DOS
#include <os2.h>
#endif
#include <termcap.h>
#include <conio.h>
#include "tass.h"

#define		TRUE		1
#define		FALSE		0

#define		BACKSPACE	'\b'
#define		VERY_LONG_STRING	2500

int LINES=23;
int COLS=80;

#ifdef	OS2
int inverse_okay = TRUE;
#else
int inverse_okay = FALSE;
#endif

/*
#ifdef BSD
#  ifndef BSD4_1
#    include <sgtty.h>
#  else
#    include <termio.h>
#  endif
# else
#  include <termio.h>
#endif
*/

#include <ctype.h>

#define TTYIN	0

#ifdef SHORTNAMES
# define _clearinverse	_clrinv
# define _cleartoeoln	_clrtoeoln
# define _cleartoeos	_clr2eos
#endif

#ifndef	AMIGA
#ifndef OS2
#ifndef BSD
struct termio _raw_tty, 
              _original_tty;
#else
#define TCGETA	TIOCGETP
#define TCSETAW	TIOCSETP

struct sgttyb _raw_tty,
	      _original_tty;
#endif
#endif
#endif

static int _inraw = 0;                  /* are we IN rawmode?    */

#define DEFAULT_LINES_ON_TERMINAL	24
#define DEFAULT_COLUMNS_ON_TERMINAL	80

static int _memory_locked = 0;		/* are we IN memlock??   */
static int _line  = -1,                 /* initialize to "trash" */
           _col   = -1;
           
static int _intransmit;			/* are we transmitting keys? */

static
char *_clearscreen, *_moveto, *_cleartoeoln, *_cleartoeos,
	*_setinverse, *_clearinverse;

static
int _lines,_columns;

static char _terminal[1024];              /* Storage for terminal entry */
static char _capabilities[1024];           /* String for cursor motion */

static char *ptr = _capabilities;	/* for buffering         */

int    outchar();			/* char output for tputs */
#if 0
char  *tgetstr(),     		       /* Get termcap capability */
      *tgoto();				/* and the goto stuff    */

extern int tgetent();      /* get termcap entry */
#endif
extern char *strcpy(); extern char *getenv();

InitScreen()
{
int  err;
char termname[40];
	
	if (getenv("TERM") == NULL) {
		fprintf(stderr,
		  "TERM variable not set; Tass requires screen capabilities\n");
		return(FALSE);
	}
	if (strcpy(termname, getenv("TERM")) == NULL) {
		fprintf(stderr,"Can't get TERM variable\n");
		return(FALSE);
	}
	if ((err = tgetent(_terminal, termname)) != 1) {
		fprintf(stderr,"Can't get entry for TERM\n");
		return(FALSE);
	}

        _line  =  0;            /* where are we right now?? */
        _col   =  0;            /* assume zero, zero...     */

	/* load in all those pesky values */
	_clearscreen       = tgetstr("cl", &ptr);
	_moveto            = tgetstr("cm", &ptr);
	_cleartoeoln       = tgetstr("ce", &ptr);
	_cleartoeos        = tgetstr("cd", &ptr);
	_lines	      	   = tgetnum("li");
	_columns	   = tgetnum("co");
	_setinverse        = tgetstr("so", &ptr);
	_clearinverse      = tgetstr("se", &ptr);

	if (!_clearscreen) {
		fprintf(stderr,
			"Terminal must have clearscreen (cl) capability\n");
		return(FALSE);
	}
	if (!_moveto) {
		fprintf(stderr,
			"Terminal must have cursor motion (cm)\n");
		return(FALSE);
	}
	if (!_cleartoeoln) {
		fprintf(stderr,
			"Terminal must have clear to end-of-line (ce)\n");
		return(FALSE);
	}
#ifndef OS2
	if (!_cleartoeos) {
		fprintf(stderr,
			"Terminal must have clear to end-of-screen (cd)\n");
		return(FALSE);
	}
#endif
	if (_lines == -1)
		_lines = DEFAULT_LINES_ON_TERMINAL;
	if (_columns == -1)
		_columns = DEFAULT_COLUMNS_ON_TERMINAL;

        tputs(tgetstr("ti", &ptr), 1, outchar);
        fflush(stdout);

	return(TRUE);
}

ExitScreen()
{
	tputs(tgetstr("te", &ptr), 1, outchar);
        fflush(stdout);
}

ScreenSize(lines, columns)
int *lines, *columns;
{
	/** returns the number of lines and columns on the display. **/

#ifdef OS2
	VIOMODEINFO vmi;
	vmi.cb = sizeof(vmi);
	VioGetMode(&vmi, 0);
	_lines = vmi.row;
	_columns = vmi.col;
#endif

	if (_lines == 0) _lines = DEFAULT_LINES_ON_TERMINAL;
	if (_columns == 0) _columns = DEFAULT_COLUMNS_ON_TERMINAL;

	*lines = _lines - 1;		/* assume index from zero*/
	*columns = _columns;		/* assume index from one */
}

ClearScreen()
{
	/* clear the screen: returns -1 if not capable */

	tputs(_clearscreen, 1, outchar);
	fflush(stdout);      /* clear the output buffer */
}

MoveCursor(row, col)
int row, col;
{
	/** move cursor to the specified row column on the screen.
            0,0 is the top left! **/

	char *stuff, *tgoto();

        _line = row;    /* to ensure we're really there... */
        _col  = col;
                
	stuff = tgoto(_moveto, col, row);
	tputs(stuff, 1, outchar);
	fflush(stdout);
}

CleartoEOLN()
{
	/** clear to end of line **/

	tputs(_cleartoeoln, 1, outchar);
	fflush(stdout);  /* clear the output buffer */
}

CleartoEOS()
{
	/** clear to end of screen **/
        if (!_cleartoeos)
#ifdef OS2
        {
		int i;
        	printf("\033[s\033[K");
        	for ( i = _line + 1; i < _lines; i++ )
        		printf("\033[%d;1H\033[K", i + 1);
        	        printf("\033[u");
        	        fflush(stdout);
        }
#else
                return(-1);
#endif


	tputs(_cleartoeos, 1, outchar);
	fflush(stdout);  /* clear the output buffer */
	return (0);
}

StartInverse()
{
	/** set inverse video mode **/

	if (_setinverse && inverse_okay)
		tputs(_setinverse, 1, outchar);
/*	fflush(stdout);	*/
}


EndInverse()
{
	/** compliment of startinverse **/

	if (_clearinverse && inverse_okay)
		tputs(_clearinverse, 1, outchar);
/*	fflush(stdout);	*/
}

RawState()
{
	/** returns either 1 or 0, for ON or OFF **/

	return( _inraw );
}

Raw(state)
int state;
{
	/** state is either TRUE or FALSE, as indicated by call **/

#ifndef OS2	
#ifdef	AMIGA
	long raw (), cooked ();
	if (state == FALSE && _inraw) {
		cooked ();
		_inraw = 0;
	} else if (state == TRUE && !_inraw) {
		raw ();
		_inraw = 1;
	}
#else
	if (state == FALSE && _inraw) {
	  (void) ioctl(TTYIN, TCSETAW, &_original_tty);
	  _inraw = 0;
	}
	else if (state == TRUE && ! _inraw) {

	  (void) ioctl(TTYIN, TCGETA, &_original_tty);	/** current setting **/

	  (void) ioctl(TTYIN, TCGETA, &_raw_tty);    /** again! **/
#ifdef BSD
	  _raw_tty.sg_flags &= ~(ECHO | CRMOD);	/* echo off */
	  _raw_tty.sg_flags |= CBREAK;	/* raw on    */
#else
	  _raw_tty.c_lflag &= ~(ICANON | ECHO);	/* noecho raw mode        */

	  _raw_tty.c_cc[VMIN] = '\01';	/* minimum # of chars to queue    */
	  _raw_tty.c_cc[VTIME] = '\0';	/* minimum time to wait for input */

#endif
#endif
	  (void) ioctl(TTYIN, TCSETAW, &_raw_tty);

	  _inraw = 1;
	}
#else
/* OS2 */
	if (state == FALSE && _inraw) {
		_inraw = 0;
	} else if (state == TRUE && !_inraw) {
		_inraw = 1;
	}
#endif
}

int
ReadCh()
{
	/** read a character with Raw mode set! **/

	register int result;
	char ch;
#ifdef OS2
#define K_LEFT	+75
#define K_RIGHT	+77
#define K_UP	+72
#define K_DOWN	+80
#define K_PGUP	+73
#define K_PGDN	+81
#define K_HOME	+71
#define K_END	+79

	while (!(result = getch ()))	{	/* no funnies... */
		switch (result = getch ()) {
		case K_UP:
			result = 'k';
			break;
		case K_DOWN:
			result = 'j';
			break;
		case K_PGDN:
			result = ctrl('D');
			break;
		case K_PGUP:
			result = ctrl('U');
			break;
		case K_LEFT:
			result = '<';
			break;
		case K_RIGHT:
			result = '>';
			break;
		default:
			continue;	/* ignore */
		}
		break;
	}
	ch = (char) result;
#else
	result = read(0, &ch, 1);
#endif
        return((result <= 0 ) ? EOF : ch & 0x7F);
}


outchar(c)
char c;
{
	/** output the given character.  From tputs... **/
	/** Note: this CANNOT be a macro!              **/

	putc(c, stdout);
}


draw(p, q, n)
char *p;
char *q;
int n;
{
	int i;
	extern char cstate[];

	MoveCursor(4, 0);

	for (; *p; p++, q++)
		for (i = n; i < *q; i++) {
			if (*p == cstate[2])
				fputs("\r\n", stdout);
			else
				putchar(*p);
		}
}

