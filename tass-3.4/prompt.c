
#include	<stdio.h>
#include	"tass.h"


/*
 *  parse_num
 *  get a number from the user
 *  Return -1 if missing or bad number typed
 */

parse_num(ch, prompt)
char ch;
char *prompt;
{
	char buf[40];
	int len;
	int i;
	int num;

	MoveCursor(LINES,0);
	printf("%s %c",prompt,ch);
	fflush(stdout);
	buf[0] = ch;
	buf[1] = '\0';
	len = 1;
	ch = ReadCh();
	while (ch != '\n'&& ch != '\r') {
		if (ch == 8 || ch == 127) {
			if (len) {
				len--;
				buf[len] = '\0';
				putchar('\b');
				putchar(' ');
				putchar('\b');
			} else {
				MoveCursor(LINES, 0);
				CleartoEOLN();
				return(-1);
			}
		} else if (ch == 21) {	/* control-U	*/
			for (i = len;i>0;i--) {
				putchar('\b');
				putchar(' ');
				putchar('\b');
			}
			buf[0] = '\0';
			len = 0;
		} else if (ch >= '0' && ch <= '9' && len < 4) {
			buf[len++] = ch;
			buf[len] = '\0';
			putchar(ch);
		} else
			putchar(7);
		fflush(stdout);
		ch = ReadCh();
	}

	MoveCursor(LINES, 0);
	CleartoEOLN();

	if (len) {
		num = atoi(buf);
		return(num);
	} else
		return(-1);
}


/*
 *  parse_string
 *  get a string from the user
 *  Return TRUE if a valid string was typed, FALSE otherwise
 */

parse_string(prompt, buf)
char *prompt;
char *buf;
{
int len;
int i;
char ch;

	clear_message();
	MoveCursor(LINES,0);
	printf("%s", prompt);
	fflush(stdout);
	buf[0] = '\0';
	len = 0;
	ch = ReadCh();
	while (ch != '\n' && ch != '\r') {
		if (ch == 8 || ch == 127) {
			if (len) {
				len--;
				buf[len] = '\0';
				putchar('\b');
				putchar(' ');
				putchar('\b');
			} else {
				MoveCursor(LINES, 0);
				CleartoEOLN();
				return(FALSE);
			}
		} else if (ch == 21) {	/* control-U	*/
			for (i = len;i>0;i--) {
				putchar('\b');
				putchar(' ');
				putchar('\b');
			}
			buf[0] = '\0';
			len = 0;
		} else if (ch >= ' ' && len < 60) {
			buf[len++] = ch;
			buf[len] = '\0';
			putchar(ch);
		} else
			putchar(7);
		fflush(stdout);
		ch = ReadCh();
	}
	MoveCursor(LINES,0);
	CleartoEOLN();

	return TRUE;
}


prompt_yn(prompt)
char *prompt;
{
	char ch;

	clear_message();
	MoveCursor(LINES,0);
	printf("%s", prompt);
	fflush(stdout);

	ch = ReadCh();
	clear_message();

	if (ch == 'y' || ch == 'Y')
		return TRUE;

	return FALSE;
}


page_cont() {
	int c;
	extern int glob_respnum;
	extern char * glob_page_group;
	extern int ch_instead;
	extern FILE *note_fp;
	extern long note_mark[];
	extern int note_page;

	printf("-Hit return to continue-");
	fflush(stdout);
	c = ReadCh();

	if (c == '\r' || c == '\n')
		redraw_page(glob_respnum, glob_page_group);
	else
	{
		ch_instead = c;
		fseek(note_fp, note_mark[note_page], 0);
	}
}


continue_prompt() {
	int c;

	printf("-Hit return to continue-");
	fflush(stdout);
	c = ReadCh();
}

