
#include	<stdio.h>
#include	<signal.h>
#include	"tass.h"


int first_group_on_screen;
int last_group_on_screen;
int cur_groupnum = 0;
extern int index_point;
int space_mode;
extern char *cvers;

char group_search_string[LEN+1];



#ifdef SIGTSTP
void
select_susp(i)
int i;
{

	Raw(FALSE);
	putchar('\n');
	signal(SIGTSTP, SIG_DFL);
#ifdef BSD
        sigsetmask(sigblock(0) & ~(1 << (SIGTSTP - 1)));
#endif
	kill(0, SIGTSTP);

	signal(SIGTSTP, select_susp);
	Raw(TRUE);
	mail_setup();
	group_selection_page();
}
#endif


selection_index()
{
	char ch;
	int n;
	int i;
	char buf[LEN];

	group_selection_page();		/* display group selection page */

	while (1) {
		ch = ReadCh();

		if (tass_backwards)
			ch = switch_chars(ch);

		if (ch > '0' && ch <= '9') {
			prompt_group_num(ch);
		} else switch (ch) {
			case 'c':	/* catchup--mark all articles as read */
			    if (prompt_yn("Mark group as read? (y/n): ")) {
				unread[cur_groupnum] = 0;
				mark_group_read(
					    active[my_group[cur_groupnum]].name,
					    my_group[cur_groupnum]);
 				MoveCursor(INDEX_TOP + (cur_groupnum -
						first_group_on_screen), 47);
 				printf("     ");
				MoveCursor(LINES, 0);
 				fflush(stdout);
/*				group_selection_page();		*/
			    }
			    break;

			case ctrl('K'):
				if (local_top <= 0) {
					info_message("No groups to delete");
					break;
				}

				delete_group(
					active[my_group[cur_groupnum]].name);
				active[my_group[cur_groupnum]].flag = NOTGOT;

				local_top--;
				for (i = cur_groupnum; i < local_top; i++) {
					my_group[i] = my_group[i+1];
					unread[i] = unread[i+1];
				}
				if (cur_groupnum >= local_top)
					cur_groupnum = local_top - 1;

				group_selection_page();
				break;

			case ctrl('Y'):
				undel_group();
				group_selection_page();
				break;

			case 'I':		/* toggle inverse video */
				inverse_okay = !inverse_okay;
				if (inverse_okay)
					info_message("Inverse video enabled");
				else
					info_message("Inverse video disabled");
				break;

			case ctrl('R'):	/* reset .newsrc */
			    if (prompt_yn("Reset newsrc? (y/n): ")) {
				reset_newsrc();
				cur_groupnum = 0;
				group_selection_page();
			    }
			    break;

			case '$':	/* reread .newsrc, no unsub groups */
				cur_groupnum = 0;
				local_top = 0;
				for (i = 0; i < num_active; i++)
					active[i].flag = NOTGOT;
				read_newsrc(TRUE);
				group_selection_page();
				break;

			case 's':	/* subscribe to current group */
			    MoveCursor(INDEX_TOP +
				(cur_groupnum-first_group_on_screen), 3);
			    putchar(' ');
			    fflush(stdout);
			    MoveCursor(LINES, 0);

			    subscribe(active[my_group[cur_groupnum]].name,
					':', my_group[cur_groupnum], FALSE);
			    sprintf(buf, "subscribed to %s",
					active[my_group[cur_groupnum]].name);
			    info_message(buf);
			    break;

			case 'u':	/* unsubscribe to current group */
			    MoveCursor(INDEX_TOP +
				(cur_groupnum-first_group_on_screen), 3);
			    putchar('u');
			    fflush(stdout);
			    MoveCursor(LINES, 0);

			    subscribe(active[my_group[cur_groupnum]].name,
					'!', my_group[cur_groupnum], FALSE);
			    sprintf(buf, "unsubscribed to %s",
					active[my_group[cur_groupnum]].name);
			    info_message(buf);
			    break;

			case '\t':
				for (i = cur_groupnum; i < local_top; i++)
					if (unread[i] != 0)
						break;
				if (i >= local_top) {
					info_message("No more groups to read");
					break;
				}

				erase_group_arrow();
				cur_groupnum = i;
				if (cur_groupnum >= last_group_on_screen)
					group_selection_page();
				else
					draw_group_arrow();
				space_mode = TRUE;
				goto go_into_group;

			case 'g':	/* prompt for a new group name */
				n = choose_new_group();
				if (n >= 0) {
					erase_group_arrow();
					cur_groupnum = n;
					if (cur_groupnum < first_group_on_screen
					|| cur_groupnum >= last_group_on_screen)
						group_selection_page();
					else
						draw_group_arrow();
				}
				break;

			case 27:	/* (ESC) common arrow keys */
				ch = ReadCh();
				n = 1;
				if (ch == '[' || ch == 'O')
					ch = ReadCh();
				*cstate = ch;

				switch (ch) {
				case 'A':
				case 'D':
				case 'i':
					goto select_up;

				case 'B':
				case 'I':
				case 'C':
					goto select_down;

				case 'b':
				case 'f':
				case 's':
					if (h[n+1] - ch)
						break;
					else {
						cstate[n] = ReadCh();
						ff1(n, cstate[n] - 'm');
						group_selection_page();
					}
				}
				break;

			case 'y':	/* pull in rest of groups from active */
				n = local_top;
				for (i = 0; i < num_active; i++)
					active[i].flag = NOTGOT;
				read_newsrc(FALSE);
				for (i = 0; i < num_active; i++)
					if (active[i].flag & NOTGOT) {
						active[i].flag &= ~NOTGOT;
						my_group[local_top] = i;
						unread[local_top] = -1;
						local_top++;
					}
				if (n < local_top) {
					sprintf(buf, "Added %d group%s",
						local_top - n,
						local_top - n == 1 ? "" : "s");
					group_selection_page();
					info_message(buf);
				} else
				    info_message("No more groups to yank in");
				break;

			case ctrl('U'):		/* page up */
			case ctrl('B'):		/* full page up */
				erase_group_arrow();
				cur_groupnum -=
				    NOTESLINES / (ch == ctrl('U') ? 2 : 1);
				if (cur_groupnum < 0)
					cur_groupnum = 0;
				if (cur_groupnum < first_group_on_screen
				||  cur_groupnum >= last_group_on_screen)
					group_selection_page();
				else
					draw_group_arrow();
				break;

			case ctrl('D'):		/* page down */
			case ctrl('F'):		/* full page down */
				erase_group_arrow();
				cur_groupnum += 
				    NOTESLINES / (ch == ctrl('D') ? 2 : 1);
				if (cur_groupnum >= local_top)
					cur_groupnum = local_top - 1;

				if (cur_groupnum <= first_group_on_screen
				||  cur_groupnum >= last_group_on_screen)
					group_selection_page();
				else
					draw_group_arrow();
				break;

			case '!':
				shell_escape();
				group_selection_page();
				break;

			case 'v':
				info_message(cvers);
				break;

			case ctrl('N'):		/* line down */
			case 'j':
select_down:
				if (cur_groupnum + 1 >= local_top)
					break;

				if (cur_groupnum + 1 >= last_group_on_screen) {
					cur_groupnum++;
					group_selection_page();
				} else {
					erase_group_arrow();
					cur_groupnum++;
					draw_group_arrow();
				}
				break;

			case ctrl('P'):		/* line up */
			case 'k':
select_up:
				if (!cur_groupnum)
					break;

				if (cur_groupnum <= first_group_on_screen) {
					cur_groupnum--;
					group_selection_page();
				} else {
					erase_group_arrow();
					cur_groupnum--;
					draw_group_arrow();
				}
				break;

			case 't':		/* redraw */
			case ctrl('W'):
			case ctrl('L'):
				group_selection_page();
				break;

			case ' ':	/* see TASS_BACKWARDS before changing */
			case '\r':	/* go into group */
			case '\n':
				space_mode = FALSE;
go_into_group:
				clear_message();
				index_point = -1;
				do {
					group_page(
					  active[my_group[cur_groupnum]].name);
				} while (index_point == -3);
				group_selection_page();
				break;

			case '/':	/* search forward */
				search_group(TRUE);
				break;

			case '?':	/* search backward */
				search_group(FALSE);
				break;

			case 'q':	/* quit */
				tass_done(0);

			case 'h':
				tass_select_help();
				group_selection_page();
				break;

			default:
			    info_message("Bad command.  Type 'h' for help.");
		}
	}
}


group_selection_page() {
	int i;
	int n;
	char new[10];
	char subs;

#ifdef SIGTSTP
	signal(SIGTSTP, select_susp);
#endif

	ClearScreen();
	printf("%s\r\n", nice_time());		/* print time in upper left */

	if (mail_check()) {			/* you have mail message */
		MoveCursor(0, COLS-18);		/* in upper right */
		printf("you have mail\n");
	}

	center_line(1, "Group Selection");
	MoveCursor(INDEX_TOP, 0);

	first_group_on_screen = (cur_groupnum / NOTESLINES) * NOTESLINES;

	last_group_on_screen = first_group_on_screen + NOTESLINES;
	if (last_group_on_screen >= local_top)
		last_group_on_screen = local_top;

	for (i = first_group_on_screen; i < last_group_on_screen; i++) {
		switch (unread[i]) {
		case -2:
			strcpy(new, "?   ");
			break;

		case -1:
			strcpy(new, "-   ");
			break;

		case 0:
			strcpy(new, "    ");
			break;

		default:
			sprintf(new, "%-4d", unread[i]);
		}

		n = my_group[i];
		if (active[n].flag & SUBS)	/* subscribed? */
			subs = ' ';
		else
			subs = 'u';	/* u next to unsubscribed groups */

		printf("   %c %4d  %-35s %s\r\n", subs, i+1,
							active[n].name, new);
	}

	draw_group_arrow();
}


prompt_group_num(ch)
char ch;
{
int num;

	clear_message();

	if ((num = parse_num(ch, "Select group> ")) == -1) {
		clear_message();
		return FALSE;
	}
	num--;		/* index from 0 (internal) vs. 1 (user) */

	if (num >= local_top)
		num = local_top - 1;

	if (num >= first_group_on_screen
	&&  num < last_group_on_screen) {
		erase_group_arrow();
		cur_groupnum = num;
		draw_group_arrow();
	} else {
		cur_groupnum = num;
		group_selection_page();
	}

	return TRUE;
}

erase_group_arrow() {
	erase_arrow(INDEX_TOP + (cur_groupnum-first_group_on_screen) );
}

draw_group_arrow() {
	draw_arrow(INDEX_TOP + (cur_groupnum-first_group_on_screen) );
}

search_group(forward)
int forward;
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
		strcpy(group_search_string, buf);
	else if (!strlen(group_search_string)) {
		info_message("No search string");
		return;
	}

	i = cur_groupnum;

	make_lower(group_search_string, buf);
	len = strlen(buf);

	do {
		if (forward)
			i++;
		else
			i--;

		if (i >= local_top)
			i = 0;
		if (i < 0)
			i = local_top - 1;

		make_lower(active[my_group[i]].name, buf2);
		if (match(buf, buf2, len)) {
			if (i >= first_group_on_screen
			&&  i < last_group_on_screen) {
				erase_group_arrow();
				cur_groupnum = i;
				draw_group_arrow();
			} else {
				cur_groupnum = i;
				group_selection_page();
			}
			return;
		}
	} while (i != cur_groupnum);

	info_message("No match");
}


choose_new_group() {
	char buf[LEN+1];
	char *p;
	int ret;

	if (!parse_string("Newsgroup> ", buf))
		return -1;

	for (p = buf; *p && (*p == ' ' || *p == '\t'); p++) ;
	if (*p == '\0')
		return -1;

	ret = add_group(p, TRUE);
	if (ret < 0)
		info_message("Group not found in active file");

	return ret;
}


/*
 *  Add a group to the selection list (my_group[])
 *  Return the index of my_group[] if group is added or was already
 *  there.  Return -1 if named group is not in active[].
 */

add_group(s, get_unread)
char *s;
int get_unread;			/* look in .newsrc for sequencer unread info? */
{
	long h;
	int i, j;
	extern long hash_groupname();

	h = hash_groupname(s);

	for (i = group_hash[h]; i >= 0; i = active[i].next)
		if (strcmp(s, active[i].name) == 0) {
			for (j = 0; j < local_top; j++)
				if (my_group[j] == i)
					return j;

			active[i].flag &= ~NOTGOT;   /* mark that we got it */
			my_group[local_top] = i;

			if (get_unread)
				unread[local_top] = get_line_unread(s, i);
			else
				unread[local_top] = -2;

			local_top++;
			return local_top - 1;
		}

	return -1;
}


