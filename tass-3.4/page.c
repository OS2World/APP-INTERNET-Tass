
#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#ifdef	OS2
#include	<assert.h>
#endif
#include	<sys/types.h>
#include	"tass.h"


#define		MAX_PAGES	1000
#define		NOTE_UNAVAIL	-1

char note_h_path[LEN];			/* Path:	*/
char note_h_date[LEN];			/* Date:	*/
char note_h_subj[LEN];			/* Subject:	*/
char note_h_from[LEN];			/* From:	*/
char note_h_reply[LEN];			/* Reply-To:	*/
char note_h_org[LEN];			/* Organization: */
char note_h_newsgroups[LEN];		/* Newsgroups:	*/
char note_h_messageid[LEN];		/* Message-ID:	*/
char note_h_distrib[LEN];		/* Distribution: */
char note_h_followup[LEN];		/* Followup-To: */

int	note_line;
int	note_page;		/* what page we're on */
long	note_mark[MAX_PAGES];	/* ftells on beginnings of pages */
FILE	*note_fp;		/* the body of the current article */
int	note_end;		/* we're done showing this article */
int	rotate;			/* 0=normal, 13=rot13 decode */
static int indent;              /* line offset */
static int wordwrap = 1;        /* word wrap */

long	note_size;		/* stat size in bytes of article */

char	note_full_name[LEN];
char	note_from_addr[LEN];

#ifdef OS2
extern INFO my_stuff;
#endif

int ch_instead = 0;


int last_resp;		/* current & previous article for - command */
int this_resp;

int glob_respnum;
char *glob_page_group;
extern int cur_groupnum;

#ifdef OS2
extern char mailer[];
#endif

#ifdef SIGTSTP
void
page_susp(i)
int i;
{

	Raw(FALSE);
	putchar('\n');
	signal(SIGTSTP, SIG_DFL);
#ifdef BSD
        sigsetmask(sigblock(0) & ~(1 << (SIGTSTP - 1)));
#endif
	kill(0, SIGTSTP);

	signal(SIGTSTP, page_susp);
	mail_setup();
	Raw(TRUE);
	redraw_page(glob_respnum, glob_page_group);
}
#endif


show_page(respnum, group, group_path)
int respnum;
char *group;
char *group_path;
{
	char ch;
	int n;
	int i;
	long art;

restart:

	glob_respnum = respnum;
	glob_page_group = group;
	indent = 0;

#ifdef SIGTSTP
	signal(SIGTSTP, page_susp);
#endif

	if (respnum != this_resp) {	   /* remember current & previous */
		last_resp = this_resp;	   /* articles for - command */
		this_resp = respnum;
	}

	rotate = 0;			/* normal mode, not rot13 */
	art = arts[respnum].artnum;
	arts[respnum].unread = 0;	/* mark article as read */
	open_note(art, group_path);

	if (note_page == NOTE_UNAVAIL) {
		ClearScreen();
		printf("[Article %ld unvailable]\r\r", art);
		fflush(stdout);
	} else
		show_note_page(respnum, group);

	while (1) {
		if (ch_instead)
		{
			ch = ch_instead;
			ch_instead = 0;
		} else
			ch = ReadCh();

		if (tass_backwards)
			ch = switch_chars(ch);

		if (ch >= '0' && ch <= '9') {

			n = prompt_response(ch, respnum);
			if (n != -1) {
				respnum = n;
				goto restart;
			}

		} else switch (ch) {
			case 'a':	/* author search forward */
			case 'A':	/* author search backward */
				i = (ch == 'a');
				n = search_author(respnum, i, group);
				if (n < 0)
					break;

				respnum = n;
				goto restart;
				break;

#ifdef XTRA
			case 'B':	/* body search */
				n = search_body(respnum, group, group_path);
				if (n < 0)
					break;

				respnum = n;
				goto restart;
				break;
#endif

			case '|':	/* pipe article into command */
				pipe_article();
				break;

			case 'I':	/* toggle inverse video */
				inverse_okay = !inverse_okay;
				if (inverse_okay)
					info_message("Inverse video enabled");
				else
					info_message("Inverse video disabled");
				goto pager_ctrlr;
				break;

			case 'o':
				archive_article(group_path);
				break;

			case 'O':
				archive_thread(respnum, group_path);
				break;

			case 's':
				save_art_to_file();
				break;

			case 'S':
				save_thread_to_file(respnum, group_path);
				break;

			case ctrl('X'):
			case '%':	/* toggle rot-13 mode */
				if (rotate)
					rotate = 0;
				else
					rotate = 13;
				redraw_page(respnum, group);
				break;

			case 'P':	/* previous unread article */
				n = prev_unread(prev_response(respnum));
				if (n == -1)
				    info_message("No previous unread article");
				else {
					note_cleanup();
					respnum = n;
					goto restart;
				}
				break;

			case 'C':	/* cancel article */
				if (!post_cancel(group))
					break;
				goto reindex_group;

			case 'F':	/* post a followup to this article */
			case 'f':
				if (!post_response(group, (ch == 'F'))) {
					redraw_page(respnum, group);
					break;
				}

reindex_group:
				update_newsrc(group, my_group[cur_groupnum]);
				n = which_base(respnum);
				note_cleanup();
				index_group(group, group_path);
				read_newsrc_line(group);
				if (n >= top_base)
					return top_base - 1;
				respnum = choose_resp(n, nresp(n));
				goto restart;

			case 'z':	/* mark article as unread (to return) */
				arts[respnum].unread = 2;
				info_message("Article marked as unread");
				break;

			case 'K':	/* mark rest of thread as read */
				for (n = respnum; n >= 0; n = arts[n].thread)
					arts[n].unread = 0;
				n = next_unread(next_response(respnum));
				if (n == -1)
					goto return_to_index;
				else {
					note_cleanup();
					respnum = n;
					goto restart;
				}
				break;

			case 'i':	/* return to index page */
return_to_index:
				note_cleanup();
				return( which_base(respnum) );

			case 't':	/* return to group selection page */
				note_cleanup();
				return -2;

			case ctrl('R'):	  /* redraw beginning of article */
pager_ctrlr:
				if (note_page == NOTE_UNAVAIL) {
					ClearScreen();
					printf("[Article %ld unvailable]\r\n",
							arts[respnum].artnum);
					fflush(stdout);
				} else {
					note_page = 0;
					note_end = FALSE;
					fseek(note_fp, note_mark[0], 0);
					show_note_page(respnum, group);
				}
				break;

			case ctrl('W'):
				wordwrap = 1 - wordwrap;
                                goto pager_ctrlr;
				break;
			
			case ctrl('L'):
				redraw_page(respnum, group);
				break;

			case '!':
				shell_escape();
				redraw_page(respnum, group);
				break;

			case '\b':
			case 'b':	/* back a page */
				if (note_page == NOTE_UNAVAIL
				||  note_page <= 1) {
					note_cleanup();
					n = prev_response(respnum);
					if (n == -1)
						return( which_resp(respnum) );

					respnum = n;
					goto restart;

				} else {
					note_page -= 2;
					note_end = FALSE;
					fseek(note_fp, note_mark[note_page], 0);
					show_note_page(respnum, group);
				}
				break;

			case 'm':	/* mail article to somebody */
				if (!mail_to_someone(FALSE))
					redraw_page(respnum, group);
				break;

			case 'w':	/* post/mail article to somebody */
			case 'W':
			        if (!post_cc_response(group, ch == 'W')) {
					redraw_page(respnum, group);
					break;
			        }
			
			        goto reindex_group;

			case 'M':	/* bounce article to somebody */
				if (!mail_to_someone(TRUE))
					redraw_page(respnum, group);
				break;

			case 'r':	/* reply to author through mail */
			case 'R':
				if (!mail_to_author(ch == 'R'))
					redraw_page(respnum, group);
				break;

			case '-':	/* show last viewed article */
				if (last_resp < 0) {
					info_message("No last message");
					break;
				}
				note_cleanup();
				respnum = last_resp;
				goto restart;

			case 'p':	/* previous article */
				note_cleanup();
				n = prev_response(respnum);
				if (n == -1)
					return( which_resp(respnum) );

				respnum = n;
				goto restart;

			case 'n':	/* skip to next article */
				note_cleanup();
				n = next_response(respnum);
				if (n == -1)
					return( which_base(respnum) );

				respnum = n;
				goto restart;

			case 'k':
				if (note_page == NOTE_UNAVAIL) {
					n = next_unread(next_response(respnum));
					if (n == -1)
						return( which_base(respnum) );

					respnum = n;
					goto restart;

				} else {
					note_cleanup();
					n = next_unread(next_response(respnum));
					if (n == -1)
						return( which_base(respnum) );

					respnum = n;
					goto restart;
				}
				break;

			case '>':	/* right */
       				indent += 10;
       				if (indent > LEN - COLS)
       					indent = LEN - COLS;
       				redraw_page(respnum, group);
       				break;

			case '<':	/* left */
       				indent -= 10;
       				if (indent < 0)
       					indent = 0;
       				redraw_page(respnum, group);
       				break;
					
			case ' ': 	/* next page or response */
					/* see TASS_BACKWARDS before changing */
				if (note_page == NOTE_UNAVAIL) {
					n = next_response(respnum);
					if (n == -1)
						return( which_base(respnum) );

					respnum = n;
					goto restart;

				} else if (note_end) {
					note_cleanup();
					n = next_response(respnum);
					if (n == -1)
						return( which_base(respnum) );

					respnum = n;
					goto restart;
				} else
					show_note_page(respnum, group);
				break;

			case '\t': 	/* next page or unread response */
				if (note_page == NOTE_UNAVAIL) {
					n = next_unread(next_response(respnum));
					if (n == -1)
						return( which_base(respnum) );

					respnum = n;
					goto restart;

				} else if (note_end) {
					note_cleanup();
					n = next_unread(next_response(respnum));
					if (n == -1)
						return( which_base(respnum) );

					respnum = n;
					goto restart;
				} else
					show_note_page(respnum, group);
				break;

			case 'N':	/* next unread article */
				n = next_unread(next_response(respnum));
				if (n == -1)
					info_message("No next unread article");
				else {
					note_cleanup();
					respnum = n;
					goto restart;
				}
				break;

			case '\r':
			case '\n':	/* go to start of next thread */
				note_cleanup();
				n = next_basenote(respnum);
				if (n == -1)
					return( which_base(respnum) );

				respnum = n;
				goto restart;

			case 'q':	/* quit */
				return -4;

			case 'H':	/* show article headers */
				if (note_page == NOTE_UNAVAIL) {
					n = next_response(respnum);
					if (n == -1)
						return( which_base(respnum) );

					respnum = n;
					goto restart;
				} else {
					note_page = 0;
					note_end = FALSE;
					fseek(note_fp, 0L, 0);
					show_note_page(respnum, group);
				}
				break;


			case 'h':
				tass_page_help();
				break;

			default:
			    info_message("Bad command.  Type 'h' for help.");
		}
	}
}


note_cleanup() {

	if (note_page != NOTE_UNAVAIL)
		fclose(note_fp);
}

int	save_note_page;
int	save_note_end;
long	save_note_mark[MAX_PAGES];


note_save() {
	int i;

	save_note_page = note_page;
	save_note_end = note_end;

	for (i = 0; i <= note_page; i++)
		save_note_mark[i] = note_mark[i];

	note_cleanup();
}


note_reopen(art, group_path)
long art;
char *group_path;
{
	int i;

	open_note(art, group_path);

	if (note_page != NOTE_UNAVAIL)
	{
		note_page = save_note_page;
		note_end = save_note_end;
		for (i = 0; i <= note_page; i++)
			note_mark[i] = save_note_mark[i];
		fseek(note_fp, note_mark[note_page], 0);
	}
}


redraw_page(respnum, group)
int respnum;
char *group;
{

	if (note_page == NOTE_UNAVAIL) {
		ClearScreen();
		printf("[Article %ld unvailable]\r\r", arts[respnum].artnum);
		fflush(stdout);
	} else if (note_page > 0) {
		note_page--;
		fseek(note_fp, note_mark[note_page], 0);
		show_note_page(respnum, group);
	}
}


show_note_page(respnum, group)
int respnum;
char *group;
{
	char buf[LEN];
	char buf2[LEN+50];
	char *p;
	int i, j;
	int percent;
	int ctrl_L;		/* form feed character detected */

	ClearScreen();

	note_line = 1;

	if (note_page == 0)
		show_first_header(respnum, group);
	else
		show_cont_header(respnum);

	ctrl_L = FALSE;
	while (note_line < LINES) {
		if (fgets(buf, LEN, note_fp) == NULL) {
			note_end = TRUE;
			break;
		}

		buf[LEN-1] = '\0';
		ctrl_L = unrot(buf, buf2);

		if (wordwrap) { /* wrap lines if wordwrap flag is selected */
			j = 0;
			i = strlen (buf2);
			p = buf2;
			if (!i)
				printf ("%-*.*s\r\n", COLS, COLS, "");
			else
				while (note_line < LINES && j < i) {
	       				printf ("%-*.*s\r\n", COLS, COLS, p);
					j += COLS;
					p += COLS;
				}
		} else {
			i = indent;
			if (strlen (buf2) <= i)
	       			printf ("%-*.*s\r\n", COLS, COLS, "");
			else {
	       			p = buf2 + i;
       				i = LEN - i;
	       			if (i > COLS)
       					i = COLS;
       				printf ("%-*.*s\r\n", i, i, p);
			}
		}

		note_line += (strlen(buf2) / COLS) + 1;

		if (ctrl_L)
			break;
	}

	note_mark[++note_page] = ftell(note_fp);

	MoveCursor(LINES, MORE_POS);
	if (note_end) {
		if (arts[respnum].thread != -1)
			printf("-- next response --");
		else
			printf("-- last response --");
	} else {
		if (note_size > 0) {
		    percent = note_mark[note_page] * 100 / note_size;
		    printf("--More--(%d%%)", percent);
		} else
		    printf("--More--");
	}

	fflush(stdout);
}


unrot(buf, buf2)
char *buf;
char *buf2;
{
	char *p, *q;
	int ctrl_L = FALSE;
	int i, j;

	if (rotate) {
		for (p = buf, q = buf2; *p && *p != '\n' && q<&buf2[LEN]; p++) {
			if (*p == '\b' && q > buf2) {
				q--;
			} else if (*p == 12) {		/* ^L */
				*q++ = '^';
				*q++ = 'L';
				ctrl_L = TRUE;
			} else if (*p == '\t') {
				i = q - buf2;
				j = (i|7) + 1;

				while (i++ < j)
					*q++ = ' ';
			} else if (((*p)&0x7F) < 32) {
				*q++ = '^';
				*q++ = ((*p)&0x7F) + '@';
			} else if (*p >= 'A' && *p <= 'Z')
				*q++ = 'A' + (*p - 'A' + rotate) % 26;
			else if (*p >= 'a' && *p <= 'z')
				*q++ = 'a' + (*p - 'a' + rotate) % 26;
			else
				*q++ = *p;
			}
	} else {
		for (p = buf, q = buf2; *p && *p != '\n' && q<&buf2[LEN]; p++) {
			if (*p == '\b' && q > buf2) {
				q--;
			} else if (*p == 12) {		/* ^L */
				*q++ = '^';
				*q++ = 'L';
				ctrl_L = TRUE;
			} else if (*p == '\t') {
				i = q - buf2;
				j = (i|7) + 1;

				while (i++ < j)
					*q++ = ' ';
			} else if (((*p)&0x7F) < 32) {
				*q++ = '^';
				*q++ = ((*p)&0x7F) + '@';
			} else
				*q++ = *p;
		}
	}

	*q = '\0';
	return ctrl_L;
}


show_first_header(respnum, group)
int respnum;
char *group;
{
	int whichresp;
	int x_resp;
	char buf[LEN];
	char tmp[LEN];
	int pos, i;
	int n;

	whichresp = which_resp( respnum );
	x_resp = nresp( which_base(respnum) );

	ClearScreen();

	strcpy(buf, note_h_date);
	pos = (COLS - strlen(group)) / 2;
	for (i = strlen(buf); i < pos; i++)
		buf[i] = ' ';
	buf[i] = '\0';

	strcat(buf, group);

	for (i = strlen(buf); i < RIGHT_POS; i++)
		buf[i] = ' ';
	buf[i] = '\0';

	printf("%sNote %3d of %3d\r\n", buf, which_base(respnum) + 1, top_base);

	sprintf(buf, "Article %ld  ", arts[respnum].artnum);
	n = strlen(buf);
	fputs(buf, stdout);

	pos = (COLS - strlen( note_h_subj )) / 2 - 2;

	if (pos > n)
		MoveCursor(1, pos);
	else
		MoveCursor(1, n);

	StartInverse();
	strcpy(buf, note_h_subj);
	buf[RIGHT_POS - 2 - n] = '\0';
	fputs(buf, stdout);
	EndInverse();

	MoveCursor(1, RIGHT_POS);
	if (whichresp)
		printf("Resp %3d of %3d\r\n", whichresp, x_resp);
	else {
		if (x_resp == 0)
			printf("No responses\r\n");
		else if (x_resp == 1)
			printf("1 Response\r\n");
		else
			printf("%d Responses\r\n", x_resp);
	}

	if (*note_h_org)
		sprintf(tmp, "%s at %s", note_full_name, note_h_org);
	else
		strcpy(tmp, note_full_name);

	tmp[79] = '\0';

	sprintf(buf, "%s  ", note_from_addr);

	pos = COLS - 1 - strlen(tmp);
	if (strlen(buf) + strlen(tmp) >= COLS - 1) {
		strncat(buf, tmp, COLS - 1 - strlen(buf));
		buf[COLS - 1] = '\0';
	} else {
		for (i = strlen(buf); i < pos; i++)
			buf[i] = ' ';
		buf[i] = '\0';
		strcat(buf, tmp);
	}
	printf("%s\r\n\r\n", buf);

	note_line += 4;
}


show_cont_header(respnum)
int respnum;
{
	int whichresp;
	int whichbase;
	char buf[LEN];

	whichresp = which_resp(respnum);
	whichbase = which_base(respnum);

	assert (whichbase < top_base);

	if (whichresp)
		sprintf(buf, "Note %d of %d, Resp %d (page %d):  %s",
			whichbase + 1,
			top_base,
			whichresp,
			note_page + 1,
			note_h_subj);
	else
		sprintf(buf, "Note %d of %d (page %d):  %s",
			whichbase + 1,
			top_base,
			note_page + 1,
			note_h_subj);

	buf[COLS] = '\0';
	printf("%s\r\n\r\n", buf);

	note_line += 2;
}


open_note(art, group_path)
long art;
char *group_path;
{
	char buf[4096+1];
	char *p;
	extern FILE *open_art_fp();

	note_page = 0;

	note_fp = open_art_fp(group_path, art);
	if (note_fp == NULL) {
		note_page = NOTE_UNAVAIL;
		return;
	}

	note_h_from[0] = '\0';
	note_h_reply[0] = '\0';
	note_h_path[0] = '\0';
	note_h_subj[0] = '\0';
	note_h_org[0] = '\0';
	note_h_date[0] = '\0';
	note_h_newsgroups[0] = '\0';
	note_h_messageid[0] = '\0';
	note_h_distrib[0] = '\0';
	note_h_followup[0] = '\0';

	while (fgets(buf, 4096, note_fp) != NULL) {
		buf[4096] = '\0';

		for (p = buf; *p && *p != '\n'; p++)
			if (((*p)&0x7F) < 32)
				*p = ' ';
		*p = '\0';

		if (*buf == '\0')
			break;

		if (strncmp(buf, "From: ", 6) == 0) {
			strncpy(note_h_from, &buf[6], LEN);
			note_h_from[LEN-1] = '\0';
		} else if (strncmp(buf, "Path: ", 6) == 0) {
			strncpy(note_h_path, &buf[6], LEN);
			note_h_path[LEN-1] = '\0';
		} else if (strncmp(buf, "Subject: ", 9) == 0) {
			strncpy(note_h_subj, &buf[9], LEN);
			note_h_subj[LEN-1] = '\0';
		} else if (strncmp(buf, "Organization: ", 14) == 0) {
			strncpy(note_h_org, &buf[14], LEN);
			note_h_org[LEN-1] = '\0';
		} else if (strncmp(buf, "Date: ", 6) == 0) {
			strncpy(note_h_date, &buf[6], LEN);
			note_h_date[LEN-1] = '\0';
		} else if (strncmp(buf, "Newsgroups: ", 12) == 0) {
			strncpy(note_h_newsgroups, &buf[12], LEN);
			note_h_newsgroups[LEN-1] = '\0';
		} else if (strncmp(buf, "Message-ID: ", 12) == 0) {
			strncpy(note_h_messageid, &buf[12], LEN);
			note_h_messageid[LEN-1] = '\0';
		} else if (strncmp(buf, "Distribution: ", 14) == 0) {
			strncpy(note_h_distrib, &buf[14], LEN);
			note_h_distrib[LEN-1] = '\0';
		} else if (strncmp(buf, "Reply-To: ", 10) == 0) {
			strncpy(note_h_reply, &buf[10], LEN);
			note_h_reply[LEN-1] = '\0';
		} else if (strncmp(buf, "Followup-To: ", 13) == 0) {
			strncpy(note_h_followup, &buf[13], LEN);
			note_h_followup[LEN-1] = '\0';
		}
	}

	note_page = 0;
	note_mark[0] = ftell(note_fp);

	parse_from(note_h_from, note_from_addr, note_full_name);
	note_end = FALSE;

	return;
}


prompt_response(ch, respnum)
int respnum;
{
	int num;

	clear_message();

	if ((num = parse_num(ch, "Read response> ")) == -1) {
		clear_message();
		return(-1);
	}

	return choose_resp( which_base(respnum), num );
}


/*
 *  return response number n from thread i
 */

choose_resp(i, n)
int i;
int n;
{
	int j;

	j = base[i];

	while (n-- && arts[j].thread >= 0)
		j = arts[j].thread;

	return j;
}


/*
 *  Parse various From: lines into the component mail addresses and
 *  real names
 */

parse_from(str, addr, name)
char *str;
char *addr;
char *name;
{
	char *p;

	for (p = str; *p; p++)
		if (((*p) & 0x7F) < 32)
			*p = ' ';

	while (*str && *str != ' ')
		*addr++ = *str++;
	*addr = '\0';
	if (*str++ == ' ') {
		if (*str++ == '(') {
			if (*str == '"')
				str++;  /* Kill "quotes around names"         */
					/* But don't touch quotes inside the  */
					/* Name (that's what that nonsense    */
					/* below is for			      */
			while (*str == ' ')
				str++;
			while (*str && *str != ')' && !(*str=='"'&&str[1]==')'))
				*name++ = *str++;
		}
	}
	*name = '\0';
}


yank_to_addr(orig, addr)
char *orig;
char *addr;
{
	char *p;

	for (p = orig; *p; p++)
		if (((*p) & 0x7F) < 32)
			*p = ' ';

	while (*addr)
		addr++;

	while (*orig) {
		while (*orig && (*orig == ' ' || *orig == '"' || *orig == ','))
			orig++;
		*addr++ = ' ';
		while (*orig && (*orig != ' ' && *orig != ',' && *orig != '"'))
			*addr++ = *orig++;
		while (*orig && (*orig == ' ' || *orig == '"' || *orig == ','))
			orig++;
		if (*orig == '(') {
			while (*orig && *orig != ')')
				orig++;
			if (*orig == ')')
				orig++;
		}
	}
	*addr = '\0';
}


/*
 *  Read a file grabbing the address given for To: and
 *  sticking it in mail_to
 */

find_new_to(nam, mail_to)
char *nam;
char *mail_to;
{
	FILE *fp;
	char buf[LEN];
	char buf2[LEN];
	char dummy[LEN];
	char new_mail_to[LEN];
	char *p;

	*new_mail_to = '\0';

	fp = fopen(nam, "r");
	if (fp == NULL) {
		fprintf(stderr, "reopen of %s failed\n", nam);
		return;
	}

	while (fgets(buf, 4096, fp) != NULL) {
		for (p = buf; *p && *p != '\n'; p++) ;
		*p = '\0';

		if (*buf == '\0')
			break;

		if (strncmp(buf, "To: ", 4) == 0) {
			strncpy(buf2, &buf[4], LEN);
			buf2[LEN-1] = '\0';
			yank_to_addr(buf2, new_mail_to);
		} else if (strncmp(buf, "Cc: ", 4) == 0) {
			strncpy(buf2, &buf[4], LEN);
			buf2[LEN-1] = '\0';
			yank_to_addr(buf2, new_mail_to);
		}
	}

	fclose(fp);
	if (new_mail_to[0] == ' ')
		strcpy(mail_to, &new_mail_to[1]);
	else
		strcpy(mail_to, new_mail_to);
}


save_art_to_file()
{
	char nam[LEN];
	FILE *fp;
	char *p;

	if (!parse_string("Save article to file: ", nam))
		return;
	if (nam[0] == '\0')
		return;

	for (p = nam; *p && (*p == ' ' || *p == '\t'); p++) ;
	if (!*p)
		return;

	setuid(real_uid);
	setgid(real_gid);

	if ((fp = fopen(p, "a+")) == NULL) {
		fprintf(stderr, "can't open %s: ", nam);
		perror("");
		info_message("-- article not saved --");
		setuid(real_uid);
		setgid(real_gid);
		return;
	}

	MoveCursor(LINES, 0);
	fputs("Saving...", stdout);
	fflush(stdout);

	fprintf(fp, "From %s %s\n", note_h_path, note_h_date);

	fseek(note_fp, 0L, 0);
	copy_fp(note_fp, fp, "");
	fputs("\n", fp);

	fclose(fp);

	setuid(real_uid);
	setgid(real_gid);

	fseek(note_fp, note_mark[note_page], 0);

	info_message("-- article saved --");
}


save_thread_to_file(respnum, group_path)
int respnum;
char *group_path;
{
	char nam[LEN];
	FILE *fp;
	FILE *art;
	int i;
	int b;
	int count = 0;
	char *p;

	b = which_base(respnum);

	if (!parse_string("Save thread to file: ", nam))
		return;
	if (nam[0] == '\0')
		return;

	for (p = nam; *p && (*p == ' ' || *p == '\t'); p++) ;
	if (!*p)
		return;

	setuid(real_uid);
	setgid(real_gid);

	if ((fp = fopen(nam, "a+")) == NULL) {
		fprintf(stderr, "can't open %s: ", nam);
		perror("");
		info_message("-- thread not saved --");
		setuid(real_uid);
		setgid(real_gid);
		return;
	}

	MoveCursor(LINES, 0);
	fputs("Saving...    ", stdout);
	fflush(stdout);

	note_save();

	for (i = base[b]; i >= 0; i = arts[i].thread) {
		open_note(arts[i].artnum, group_path);

		fprintf(fp, "From %s %s\n", note_h_path, note_h_date);
		fseek(note_fp, 0L, 0);
		copy_fp(note_fp, fp, "");
		fputs("\n", fp);

		note_cleanup();
		printf("\b\b\b\b%4d", ++count);
		fflush(stdout);
	}

	fclose(fp);
	setuid(real_uid);
	setgid(real_gid);

	info_message("-- thread saved --");
	note_reopen(arts[respnum].artnum, group_path);
	fseek(note_fp, note_mark[note_page], 0);
}


pipe_article() {
	char command[LEN];
	FILE *fp;

	if (!parse_string("Pipe to command: ", command))
		return;
	if (command[0] == '\0')
		return;

	fp = popen(command, "w");
	if (fp == NULL) {
		fprintf(stderr, "command failed: ");
		perror("");
		goto pipe_article_done;
	}

	fseek(note_fp, 0L, 0);
	copy_fp(note_fp, fp, "");
	pclose(fp);

pipe_article_done:

	page_cont();
}

