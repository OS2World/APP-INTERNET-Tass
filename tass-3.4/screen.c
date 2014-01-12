
#include	<stdio.h>
#include	<ctype.h>
#include	"tass.h"


char cstate[10];


info_message(msg)
char *msg;
{
	clear_message();	  /* Clear any old messages hanging around */
	center_line(LINES, msg);  /* center the message at screen bottom  */
	MoveCursor(LINES, 0);
}


clear_message()
{
	MoveCursor(LINES, 0);
	CleartoEOLN();
}


center_line(line, str)
int line;
char *str;
{
	int pos;

	pos = (COLS - strlen(str)) / 2;
	if (pos < 0) pos = 0;
	MoveCursor(line, pos);
	printf("%s", str);
	fflush(stdout);
}


draw_arrow(line)
int line;
{
	MoveCursor(line, 0);
	StartInverse ();
	printf("->");
	MoveCursor(line, COLS);
	EndInverse ();
	fflush(stdout);
	MoveCursor(LINES, 0);
}

erase_arrow(line)
int line;
{
	MoveCursor(line, 0);
	printf("  ");
	fflush(stdout);
}


ff1(n, ch)
int n;
char ch;
{
	extern char *g;

	if (ch != '\b' || ((cstate[++n] = ReadCh()) != *g))
		return;

	cstate[++n] = ReadCh();
	if (cstate[n] - cstate[n-1] == '\b')
		ff(n);
}


char *f=" _c /%\\c _ / _c _\\ | /_c _ | | _c _ / | | \\ _c | |c ^ ^c ";
char *g="cBBbBBBBJTECETBOOBDCDBOBRLBBGBBLBUHBBCBCBCBBHB`BCBB`BCBCI";


ff(n)
int n;
{
	char *p, *q;
	int i;

	ClearScreen();

	draw(f, g, cstate[n] & 85);
	for (p = h, q = g; *p; p++, q++)
		putchar(*p - *q + (cstate[n] & 85));

	MoveCursor(8, 31);
	for (i = 0; i < 4; i++)
		putchar(toupper(cstate[i]));

	MoveCursor(LINES, 0);
	ReadCh();
}

