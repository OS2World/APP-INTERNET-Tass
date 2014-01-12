
#include	<stdio.h>
#include	<signal.h>
#include	"tass.h"


int index_point;
int select_point;
int first_subj_on_screen;
int last_subj_on_screen;
char subject_search_string[LEN+1];
char author_search_string[LEN+1];
static int show_unread = 1;	 /* show only unread articles */

#ifdef XTRA
char body_search_string[LEN+1];
#endif

extern int cur_groupnum;
extern int last_resp;		/* page.c */
extern int this_resp;		/* page.c */
extern int space_mode;		/* select.c */
extern char *cvers;

char *glob_group;


#ifdef SIGTSTP
void
group_susp(i)
int i;
{

	Raw(FALSE);
	putchar('\n');
	signal(SIGTSTP, SIG_DFL);
#ifdef BSD
	sigsetmask(sigblock(0) & ~(1 << (SIGTSTP - 1)));
#endif
	kill(0, SIGTSTP);

	signal(SIGTSTP, group_susp);
	Raw(TRUE);
	mail_setup();
	show_group_page(glob_group);
}
#endif


group_page(group)
char *group;
{
	char ch;
	int i, n;
	char group_path[LEN];
	char *p;
	char buf[LEN];
	int flag;
	int sav_groupnum;

	glob_group = group;
	sav_groupnum = cur_groupnum;

	strcpy(group_path, group);		/* turn comp.unix.amiga into */
	for (p = group_path; *p; p++)		/* comp/unix/amiga */
		if (*p == '.')
			*p = '/';

	last_resp = -1;
	this_resp = -1;
	index_group(group, group_path);         /* update index file */
	read_newsrc_line(group);		/* get sequencer information */

	if (space_mode) {
		for (i = 0; i < top_base; i++)
			if (new_responses(i))
				break;
		if (i < top_base)
			index_point = i;
		else
			index_point = top_base - 1;
	} else
		index_point = top_base - 1;

	show_group_page(group);

	while (1) {
		ch = ReadCh();

		if (tass_backwards)
			ch = switch_chars(ch);

		if (ch > '0' && ch <= '9') {	/* 0 goes to basenote */
			prompt_subject_num(ch, group);
		} else switch (ch) {
			case 'a':	/* author search forward */
			case 'A':	/* author search backward */
				if (index_point < 0) {
					info_message("No articles");
					break;
				}

				i = (ch == 'a');

				n = search_author((int) base[index_point],
								i, group);
				if (n < 0)
					break;

				index_point = show_page(n, group, group_path);
				if (index_point < 0) {
					space_mode = FALSE;
					goto group_done;
				}
				show_group_page(group);
				break;

#ifdef XTRA
			case 'B':	/* search body */
				if (index_point < 0) {
					info_message("No articles");
					break;
				}

				n = search_body((int) base[index_point],
							group, group_path);
				if (n < 0)
					break;

				index_point = show_page(n, group, group_path);
				if (index_point < 0) {
					space_mode = FALSE;
					goto group_done;
				}
				show_group_page(group);
				break;
#endif

			case 'I':	/* toggle inverse video */
				inverse_okay = !inverse_okay;
				if (inverse_okay)
					info_message("Inverse video enabled");
				else
					info_message("Inverse video disabled");
				break;

			case 's':	/* subscribe to this group */
				subscribe(group, ':', my_group[cur_groupnum],
									TRUE);
				sprintf(buf, "subscribed to %s", group);
				info_message(buf);
				break;

			case 'u':	/* unsubscribe to this group */
				subscribe(group, '!', my_group[cur_groupnum],
									TRUE);
				sprintf(buf, "unsubscribed to %s", group);
				info_message(buf);
				break;

			case 'g':	/* choose a new group by name */
				n = choose_new_group();
				if (n >= 0 && n != cur_groupnum) {
					cur_groupnum = n;
					index_point = -3;
					goto group_done;
				}
				break;

			case 'c':	/* catchup--mark all articles as read */
			    if (prompt_yn("Mark everything as read? (y/n): ")) {
				for (n = 0; n < top; n++)
					arts[n].unread = 0;
				for (n = INDEX_TOP ;
					n < NOTESLINES + INDEX_TOP; n++ ) {
					MoveCursor(n, MAX_FROM + MAX_SUBJ + 15);
					putchar(' ');
				}
				fflush(stdout);
				info_message("All articles marked as read");
			    }
			    break;

			case 27:	/* common arrow keys */
				ch = ReadCh();
				if (ch == '[' || ch == 'O')
					ch = ReadCh();
				switch (ch) {
				case 'A':
				case 'D':
				case 'i':
					goto group_up;

				case 'B':
				case 'I':
				case 'C':
					goto group_down;
				}
				break;

			case 'n':	/* next group */
				clear_message();
				if (cur_groupnum + 1 >= local_top)
					info_message("No more groups");
				else {
					cur_groupnum++;
					index_point = -3;
					space_mode = FALSE;
					goto group_done;
				}
				break;

			case 'p':	/* previous group */
				clear_message();
				if (cur_groupnum <= 0)
					info_message("No previous group");
				else {
					cur_groupnum--;
					index_point = -3;
					space_mode = FALSE;
					goto group_done;
				}
				break;

			case '\t':
				space_mode = TRUE;

				if (show_unread &&
				    index_point + 1 < top_base) {
					++index_point;
					n = next_unread((int) base[index_point]);
					if (n >= 0) {
						n = which_base (n);
						if (n) {
							index_point = n;
							show_group_page(group);
							break;
						}
					}
				}
				if (show_unread)
					break;
				if (index_point < 0
				|| (n=next_unread((int) base[index_point]))<0) {
					for (i = cur_groupnum+1;
							i < local_top; i++)
						if (unread[i] > 0)
							break;
					if (i >= local_top)
						goto group_done;

					cur_groupnum = i;
					index_point = -3;
					goto group_done;
				}
				index_point = show_page(n, group, group_path);
				if (index_point < -1)
					goto group_done;
				show_group_page(group);
				break;

			case 'K':	/* mark rest of thread as read */
				if (new_responses(index_point)) {
				    for (i = base[index_point]; i >= 0;
							i = arts[i].thread)
					arts[i].unread = 0;
				    MoveCursor(INDEX_TOP +
				      (index_point - first_subj_on_screen),
						MAX_FROM + MAX_SUBJ + 15);
				    putchar(' ');
				    fflush(stdout);
				    flag = FALSE;
				} else
				    flag = TRUE;

				n = next_unread(
					next_response(base[index_point]));
				if (n < 0) {
				    if (flag)
					info_message("No next unread article");
				    else
					MoveCursor(LINES, 0);
				    break;
				}

				n = which_base(n);
				if (n < 0) {
					info_message(
					    "Internal error: K which_base < 0");
					break;
				}

				if (n >= last_subj_on_screen) {
					index_point = n;
					show_group_page(group);
				} else {
					erase_subject_arrow();
					index_point = n;
					draw_subject_arrow();
				}
				break;

			case 'N':	/* go to next unread article */
				if (index_point < 0) {
					info_message("No next unread article");
					break;
				}

				n = next_unread( (int) base[index_point]);
				if (n == -1)
					info_message("No next unread article");
				else {
					index_point =
						show_page(n, group, group_path);
					if (index_point < 0) {
						space_mode = FALSE;
						goto group_done;
					}
					show_group_page(group);
				}
				break;

			case 'P':	/* go to previous unread article */
				if (index_point < 0) {
				    info_message("No previous unread article");
				    break;
				}

				n = prev_response( (int) base[index_point]);
				n = prev_unread(n);
				if (n == -1)
				    info_message("No previous unread article");
				else {
					index_point =
						show_page(n, group, group_path);
					if (index_point < 0) {
						space_mode = FALSE;
						goto group_done;
					}
					show_group_page(group);
				}
				break;

			case 'w':	/* post a basenote */
				post_base(group);
				update_newsrc(group, my_group[cur_groupnum]);
				index_group(group, group_path);
				read_newsrc_line(group);
				index_point = top_base - 1;
				show_group_page(group);
				break;

			case 't':	/* return to group selection page */
				goto group_done;

			case ' ':	/* see TASS_BACKWARDS before changing */
			case '\r':
			case '\n':	/* read current basenote */
				if (index_point < 0) {
					info_message("*** No Articles ***");
					break;
				}
				index_point = show_page((int) base[index_point],
							group, group_path);
				if (index_point < -1) {
					space_mode = FALSE;
					goto group_done;
				}
				show_group_page(group);
				break;

			case ctrl('D'):         /* page down */
			case ctrl('F'):         /* full page down */
				if (!top_base || index_point == top_base - 1)
					break;

				erase_subject_arrow();
				index_point +=
				    NOTESLINES / (ch == ctrl('D') ? 2 : 1);
				if (index_point >= top_base)
					index_point = top_base - 1;

				if (index_point < first_subj_on_screen
				|| index_point >= last_subj_on_screen)
					show_group_page(group);
				else
					draw_subject_arrow();
				break;

			case '-':	/* go to last viewed article */
				if (this_resp < 0) {
					info_message("No last message");
					break;
				}
				index_point = show_page(this_resp,
							group, group_path);
				if (index_point < 0) {
					space_mode = FALSE;
					goto group_done;
				}
				show_group_page(group);
				break;

			case ctrl('U'):         /* page up */
			case ctrl('B'):         /* full page up */
				if (!top_base)
					break;

				erase_subject_arrow();
				index_point -=
				    NOTESLINES / (ch == ctrl('U') ? 2 : 1);
				if (index_point < 0)
					index_point = 0;
				if (index_point < first_subj_on_screen
				|| index_point >= last_subj_on_screen)
					show_group_page(group);
				else
					draw_subject_arrow();
				break;

			case 'v':
				info_message(cvers);
				break;

			case '!':
				shell_escape();
				show_group_page(group);
				break;

			case ctrl('N'):
			case 'j':		/* line down */
group_down:
				if (!top_base || index_point + 1 >= top_base)
					break;

				if (index_point + 1 >= last_subj_on_screen) {
					index_point++;
					show_group_page(group);
				} else {
					erase_subject_arrow();
					index_point++;
					draw_subject_arrow();
				}
				break;

			case ctrl('P'):
			case 'k':		/* line up */
group_up:
				if (!top_base || !index_point)
					break;

				if (index_point <= first_subj_on_screen) {
					index_point--;
					show_group_page(group);
				} else {
					erase_subject_arrow();
					index_point--;
					draw_subject_arrow();
				}
				break;

			case ctrl('T'):         /* toggle show only unread */
				show_unread = 1 - show_unread;
				break;

			case ctrl('R'):
			case ctrl('L'):
			case ctrl('W'):
			case 'i':		/* return to index */
					show_group_page(group);
					break;

			case '/':		/* forward search */
					search_subject(TRUE, group);
					break;

			case '?':		/* backward search */
					search_subject(FALSE, group);
					break;

			case 'q':		/* quit */
					index_point = -4;
					space_mode = FALSE;
					goto group_done;

			case 'h':
				tass_group_help();
				show_group_page(group);
				break;

			default:
			    info_message("Bad command.  Type 'h' for help.");
		}
	}

group_done:
	fix_new_highest(sav_groupnum);
	update_newsrc(group, my_group[sav_groupnum]);

	if (index_point == -4)
		tass_done(0);
}


/*
 *  Correct highest[] for the group selection page display since
 *  new articles may have been read or marked unread
 */

fix_new_highest(groupnum)
int groupnum;
{
	int i;
	int sum = 0;

	for (i = 0; i < top; i++)
		if (arts[i].unread)
			sum++;

	unread[groupnum] = sum;
}


show_group_page(group)
char *group;
{
	int i;
	int n;
	long skipped = 0;
	char resps[10];
	char new_resps;
	int respnum;
	char from_orig[LEN];
	char from_name[LEN];
	char from_addr[LEN];
	char subj_orig[LEN];

#ifdef SIGTSTP
	signal(SIGTSTP, group_susp);
#endif

	ClearScreen();
	printf("%s\r\n", nice_time());	/* time in upper left */
	center_line(1, group);

	if (mail_check()) {			/* you have mail message in */
		MoveCursor(0, COLS-18);         /* upper right */
		printf("you have mail\n");
	}

	MoveCursor(INDEX_TOP, 0);

	first_subj_on_screen = (index_point / NOTESLINES) * NOTESLINES;
	if (first_subj_on_screen < 0)
		first_subj_on_screen = 0;

	last_subj_on_screen = first_subj_on_screen + NOTESLINES;
	if (last_subj_on_screen >= top_base) {
		last_subj_on_screen = top_base;
		first_subj_on_screen = top_base - NOTESLINES;

		if (first_subj_on_screen < 0)
			first_subj_on_screen = 0;
	}

	for (i = first_subj_on_screen; i < last_subj_on_screen; i++) {
		if (new_responses(i))
			new_resps = '+';
		 else
			new_resps = ' ';

		n = nresp(i);
		if (n)
			sprintf(resps, "%4d", n);
		else
			strcpy(resps, "    ");

		respnum = base[i];

		strncpy(from_orig, arts[respnum].from, LEN-1);
		strncpy(subj_orig, arts[respnum].subject, MAX_SUBJ-1);

		from_orig[LEN-1] = '\0';
		parse_from(from_orig, from_addr, from_name);

		from_name[MAX_FROM-1] = '\0';
		subj_orig[MAX_SUBJ-1] = '\0';

		if (*from_name)
			sprintf(from_orig, "%s (%s)", from_name, from_addr);
		from_orig[MAX_FROM-1] = '\0';

		printf("  %4d  %-*s %s %-*s %c\r\n",
				i + 1,
				MAX_SUBJ,
				subj_orig,
				resps,
				MAX_FROM,
				from_orig,
				new_resps);
	}

	if (top_base <= 0)
		info_message("*** No Articles ***");
	else if (last_subj_on_screen == top_base)
		info_message("*** End of Articles ***");

	if (top_base > 0)
		draw_subject_arrow();
}

draw_subject_arrow() {
	draw_arrow(INDEX_TOP + (index_point-first_subj_on_screen) );
}

erase_subject_arrow() {

	erase_arrow(INDEX_TOP + (index_point-first_subj_on_screen) );
}


prompt_subject_num(ch, group)
char ch;
char *group;
{
int num;


	clear_message();

	if ((num = parse_num(ch, "Read article> ")) == -1) {
		clear_message();
		return FALSE;
	}
	num--;		/* index from 0 (internal) vs. 1 (user) */

	if (num >= top_base)
		num = top_base - 1;

	if (num >= first_subj_on_screen
	&&  num < last_subj_on_screen) {
		erase_subject_arrow();
		index_point = num;
		draw_subject_arrow();
	} else {
		index_point = num;
		show_group_page(group);
	}
}


#ifdef XTRA
search_body(current_art, group, group_path)
int current_art;
char *group;
char *group_path;
{
	char buf[LEN+1];
	char buf2[LEN+1];
	int i;
	int len;
	char *prompt;
	int count = 0;

	clear_message();

	prompt = "Body search: ";

	if (!parse_string(prompt, buf))
		return -1;

	if (strlen(buf))
		strcpy(body_search_string, buf);
	else if (!strlen(body_search_string)) {
		info_message("No search string");
		return -1;
	}

	make_lower(body_search_string, buf);
	len = strlen(buf);

	i = current_art;

	do {
		i = next_response(i);
		if (i < 0)
			i = 0;

		if (search_art_body(buf, len, arts[i].artnum, group_path))
			return i;

		if (count % 10 == 0) {
			if (count == 0)
				printf("Searching...    ");
			else
				printf("\b\b\b\b%4d", count);
			fflush(stdout);
		}
		count++;
	} while (i != current_art);

	info_message("Not found");
	return -1;
}


search_art_body(ss, len, art, group_path)
char *ss;
int len;
long art;
char *group_path;
{
	FILE *fp;
	char buf[LEN];
	extern FILE *open_art_fp();

	fp = open_art_fp(group_path, art);

	if (fp == NULL)
		return FALSE;

	while (fgets(buf, LEN, fp) != NULL)
	{
		if (*buf == '\n')
			break;
	}

	while (fgets(buf, LEN, fp) != NULL)
	{
		lcase(buf);
		if (match(ss, buf, len))
		{
			fclose(fp);
			return TRUE;
		}
	}

	fclose(fp);
	return FALSE;
}

#endif


search_author(current_art, forward, group)
int current_art;
int forward;
char *group;
{
	char buf[LEN+1];
	char buf2[LEN+1];
	int i;
	int len;
	char *prompt;

	clear_message();

	if (forward)
		prompt = "Author search forward: ";
	else
		prompt = "Author search backward: ";

	if (!parse_string(prompt, buf))
		return -1;

	if (strlen(buf))
		strcpy(author_search_string, buf);
	else if (!strlen(author_search_string)) {
		info_message("No search string");
		return -1;
	}

	make_lower(author_search_string, buf);
	len = strlen(buf);

	i = current_art;

	do {
		if (forward) {
			i = next_response(i);
			if (i < 0)
				i = 0;
		} else {
			i = prev_response(i);
			if (i < 0)
				i = choose_resp(top_base - 1,
							nresp(top_base - 1));
		}

		make_lower(arts[i].from, buf2);
		if (match(buf, buf2, len))
			return i;
	} while (i != current_art);

	info_message("No match");
	return -1;
}


search_subject(forward, group)
int forward;
char *group;
{
	char buf[LEN+1];
	char buf2[LEN+1];
	int i;
	int len;
	char *prompt;

	clear_message();

	if (forward)
		prompt = "/";
	else
		prompt = "?";

	if (!parse_string(prompt, buf))
		return;

	if (strlen(buf))
		strcpy(subject_search_string, buf);
	else if (!strlen(subject_search_string)) {
		info_message("No search string");
		return;
	}

	i = index_point;

	make_lower(subject_search_string, buf);
	len = strlen(buf);

	do {
		if (forward)
			i++;
		else
			i--;

		if (i >= top_base)
			i = 0;
		if (i < 0)
			i = top_base - 1;

		make_lower(arts[base[i]].subject, buf2);
		if (match(buf, buf2, len)) {
			if (i >= first_subj_on_screen
			&&  i < last_subj_on_screen) {
				erase_subject_arrow();
				index_point = i;
				draw_subject_arrow();
			} else {
				index_point = i;
				show_group_page(group);
			}
			return;
		}
	} while (i != index_point);

	info_message("No match");
}


/*
 *  Return the number of unread articles there are within a thread
 */

new_responses(thread)
int thread;
{
	int i;
	int sum = 0;

	for (i = base[thread]; i >= 0; i = arts[i].thread)
		if (arts[i].unread)
			sum++;

	return sum;
}

