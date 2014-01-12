/* curses.c 
 *
 * some simple functions for curses functionality
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <termcap.h>

#define		TRUE		1
#define		FALSE		0

#define		BACKSPACE	'\b'
#define		VERY_LONG_STRING	2500

int inverse_okay = TRUE;

static int _inraw = 0;		/* are we IN rawmode?    */

#define DEFAULT_LINES_ON_TERMINAL	30
#define DEFAULT_COLUMNS_ON_TERMINAL	80

static int _memory_locked = 0;	/* are we IN memlock??   */

static int _intransmit;		/* are we transmitting keys? */

static char 	*_clearscreen, *_moveto, *_cl_eoln, *_cl_eos, *_setinverse,
		*_clearinverse, *_setunderline, *_clearunderline;

static int _lines, _columns;

static char _terminal[1024];	/* Storage for terminal entry */
static char _capabilities[1024];/* String for cursor motion */

static char *ptr = _capabilities;	/* for buffering         */

int 	outchar();		/* char output for tputs */

InitScreen()
{
	int  err;
	char termname[40];
	char *getenv();

	if (getenv("TERM") == NULL) {
		fprintf(stderr,
	   "TERM variable not set; ERROR: require screen capabilities\n");
		return (FALSE);
	}
	if (strcpy(termname, getenv("TERM")) == NULL) {
		fprintf(stderr, "Can't get TERM variable\n");
		return (FALSE);
	}
	if ((err = tgetent(_terminal, termname)) != 1) {
		fprintf(stderr, "Can't get entry for TERM\n");
		return (FALSE);
	}
	/* load in all those pesky values */
	_clearscreen = tgetstr("cl", &ptr);
	_moveto = tgetstr("cm", &ptr);
	_cl_eoln = tgetstr("ce", &ptr);
	_cl_eos = tgetstr("cd", &ptr);
	_lines = tgetnum("li");
	_columns = tgetnum("co");
	_setinverse = tgetstr("so", &ptr);
	_clearinverse = tgetstr("se", &ptr);
	_setunderline = tgetstr("us", &ptr);
	_clearunderline = tgetstr("ue", &ptr);

	if (!_clearscreen) {
		fprintf(stderr,
			"Terminal must have clearscreen (cl) capability\n");
		return (FALSE);
	}
	if (!_moveto) {
		fprintf(stderr,
			"Terminal must have cursor motion (cm)\n");
		return (FALSE);
	}
	if (!_cl_eoln) {
		fprintf(stderr,
			"Terminal must have clear to end-of-line (ce)\n");
		return (FALSE);
	}
	if (!_cl_eos) {
		fprintf(stderr,
			"Terminal must have clear to end-of-screen (cd)\n");
		return (FALSE);
	}
	if (_lines == -1)
		_lines = DEFAULT_LINES_ON_TERMINAL;
	if (_columns == -1)
		_columns = DEFAULT_COLUMNS_ON_TERMINAL;
	return (TRUE);
}

ScreenSize(lines, columns)
int *lines, *columns;
{
	/** returns the number of lines and columns on the display. **/

	if (_lines == 0)
		_lines = DEFAULT_LINES_ON_TERMINAL;
	if (_columns == 0)
		_columns = DEFAULT_COLUMNS_ON_TERMINAL;

	*lines = _lines - 1;	/* assume index from zero */
	*columns = _columns;	/* assume index from one */
}

ClearScreen()
{
	/* clear the screen: returns -1 if not capable */

	tputs(_clearscreen, 1, outchar);
	fflush(stdout);		/* clear the output buffer */
}

MoveCursor(row, col)
int row, col;
{
	/** move cursor to the specified row column on the screen.
            0,0 is the top left! **/

	tputs(tgoto(_moveto, col, row), 1, outchar);
	fflush(stdout);
}

Clr2EOLN()
{
	/** clear to end of line **/

	tputs(_cl_eoln, 1, outchar);
	fflush(stdout);		/* clear the output buffer */
}

Clr2EOS()
{
	/** clear to end of screen **/

	tputs(_cl_eos, 1, outchar);
	fflush(stdout);		/* clear the output buffer */
}

StartInverse()
{
	/** set inverse video mode **/

	if (_setinverse && inverse_okay)
		tputs(_setinverse, 1, outchar);
	fflush(stdout);
}


EndInverse()
{
	/** compliment of startinverse **/

	if (_clearinverse && inverse_okay)
		tputs(_clearinverse, 1, outchar);
	fflush(stdout);	
}

StartUnderline()
{
	/** start underline mode **/

	if (!_setunderline)
		return (-1);

	tputs(_setunderline, 1, outchar);
	fflush(stdout);
	return (0);
}


EndUnderline()
{
	/** the compliment of start underline mode **/

	if (!_clearunderline)
		return (-1);

	tputs(_clearunderline, 1, outchar);
	fflush(stdout);
	return (0);
}

RawState()
{
	/** returns either 1 or 0, for ON or OFF **/

	return (_inraw);
}

Raw(state)
int state;
{
	/** state is either TRUE or FALSE, as indicated by call **/

	long raw (), cooked ();
	if (state == FALSE && _inraw) {
		cooked ();
		_inraw = 0;
	} else if (state == TRUE && !_inraw) {
		raw ();
		_inraw = 1;
	}
}

int ReadCh()
{
	/** read a character with Raw mode set! **/

	register int result;
	char ch;
	result = read(0, &ch, 1);
	return ((result <= 0) ? EOF : ch & 0x7F);
}


outchar(c)
char c;
{
	/** output the given character.  From tputs... **/
	/** Note: this CANNOT be a macro!              **/

	putc(c, stdout);
}
