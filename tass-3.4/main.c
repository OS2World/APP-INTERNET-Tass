
/*
 *  Tass, a visual Usenet news reader
 *  (c) Copyright 1990,1991,1993 by Rich Skrenta
 *
 *  Distribution agreement:
 *
 *	You may freely copy or redistribute this software, so long
 *	as there is no profit made from its use, sale, trade or
 *	reproduction.  You may not change this copyright notice,
 *	and it must be included prominently in any copy made.
 */

#include	<stdio.h>
#include	<signal.h>
#ifdef TIOCGWINSZ
#include	<termio.h>		/* for struct winsize */
#endif
#ifdef SCO_UNIX
#include	<sys/types.h>
#include	<sys/stream.h>
#include	<sys/ptem.h>
#endif
#include	"tass.h"


extern int LINES, COLS;

int max_active;
struct group_ent *active;		/* active file */
int group_hash[TABLE_SIZE];		/* group name --> active[] */
int *my_group;				/* .newsrc --> active[] */
int *unread;				/* highest art read in group */
int num_active;                         /* one past top of active */
int local_top;				/* one past top of my_group */
int update = FALSE;			/* update index files only mode */
int tass_backwards = FALSE;		/* switch key bindings for rn-ites */

struct header *arts;
long *base;
int max_art;
int top = 0;
int top_base;

int tass_uid;
int tass_gid;
int real_uid;
int real_gid;

int local_index;			/* do private indexing? */

char *cvers = "Tass 3.4x-4 (c) Copyright 1991,1993 by Rich Skrenta.  All rights reserved.";


#ifdef SIGTSTP
void
main_susp(i)
int i;
{

	Raw(FALSE);
	putchar('\n');
	signal(SIGTSTP, SIG_DFL);
	kill(0, SIGTSTP);

	signal(SIGTSTP, main_susp);
	mail_setup();
	Raw(TRUE);
}
#endif

int
main(argc, argv)
int argc;
char **argv;
{
	extern int optind, opterr;
	extern char *optarg;
	int errflag = 0;
	int i;
	int c;
	extern char group_search_string[];
	extern char author_search_string[];
	extern char subject_search_string[];
	extern char *is_remote();
	extern char *getenv();

	group_search_string[0] = '\0';
	author_search_string[0] = '\0';
	subject_search_string[0] = '\0';

	hash_init();
	for (i = 0; i < TABLE_SIZE; i++)
		group_hash[i] = -1;

#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif
#ifdef SIGTSTP
	signal(SIGTSTP, main_susp);
#endif
	if (getenv("TASS_BACKWARDS") != NULL)
		tass_backwards = TRUE;

	tass_uid = geteuid();
	tass_gid = getegid();
	real_uid = getuid();
	real_gid = getgid();

	init_selfinfo();	/* set up char *'s: homedir, newsrc, etc. */
	init_alloc();		/* allocate initial array sizes */

	if (tass_uid == real_uid) {	/* run out of someone's account */
		local_index = TRUE;	/* index in their home directory */
		mkdir(indexdir, 0755);
	} else			/* we're setuid, index in /usr/spool/news */
		local_index = FALSE;

	while ((c = getopt(argc, argv, "bf:u")) != -1) {
		switch(c) {
		case 'b':
			tass_backwards = TRUE;
			break;

		case 'f':
			strcpy(newsrc, optarg);
			break;

		case 'u':
			update = TRUE;
			break;

		case '?':
		default:
			errflag++;
		}
	}

	if (errflag) {
	    fprintf(stderr, "usage: tass [options] [newsgroups]\n");
	    fprintf(stderr, "   -b        switch key bindings for rn-ites\n");
	    fprintf(stderr, "   -f file   use file instead of $HOME/.newsrc\n");
	    fprintf(stderr, "   -u        update index files only\n");
	    exit(1);
	}

	if (!update)
		printf("%s%s\n", TASS_HEADER, is_remote());

	nntp_startup();         /* connect to server if we're using nntp */
	read_active();		/* load the active file into active[] */

	if (optind < argc) {
		while (optind < argc) {
			if (add_group(argv[optind], TRUE) < 0)
				fprintf(stderr,
					"group %s not found in active file\n",
								argv[optind]);
			optind++;
		}
	} else
		read_newsrc(TRUE);

	if (update) {			/* index file updater only */
		do_update();
		exit(0);
	}

	if (InitScreen() == FALSE) {
		fprintf(stderr,"Screen initialization failed\n");
		exit(1);
	}

	ScreenSize(&LINES, &COLS);
	Raw(TRUE);

#ifdef TIOCGWINSZ
	{
		struct winsize win;

		if (ioctl(0, TIOCGWINSZ, &win) == 0) {
			if (win.ws_row != 0)
				LINES = win.ws_row - 1;
			if (win.ws_col != 0)
				COLS = win.ws_col;
		}
	}
#endif

	mail_setup();		/* record mailbox size for "you have mail" */
	selection_index();

	tass_done(0);
}



tass_done(ret)
int ret;
{

	nntp_finish();		/* close connection if we're using nntp */
	MoveCursor(LINES, 0);
	CleartoEOLN();
	putchar('\r');
	ExitScreen();
	Raw(FALSE);
	exit(ret);
}


/*
 *  Dynamic table management
 *  These settings are memory conservative:  small initial allocations
 *  and a 50% expansion on table overflow.  A fast vm system with
 *  much memory might want to start with higher initial allocations
 *  and a 100% expansion on overflow, especially for the arts[] array.
 */

init_alloc() {

	max_active = 100;	/* initial alloc */

	active = (struct group_ent *) my_malloc(sizeof(*active) * max_active);
	my_group = (int *) my_malloc(sizeof(int) * max_active);
	unread = (int *) my_malloc(sizeof(int) * max_active);

	max_art = 300;		/* initial alloc */

	arts = (struct header *) my_malloc(sizeof(*arts) * max_art);
	base = (long *) my_malloc(sizeof(long) * max_art);
}


expand_art() {

	max_art += max_art / 2;         /* increase by 50% */

	arts = (struct header *) my_realloc(arts, sizeof(*arts) * max_art);
	base = (long *) my_realloc(base, sizeof(long) * max_art);
}


expand_active() {

	max_active += max_active / 2;		/* increase by 50% */

	active = (struct group_ent *) my_realloc(active,
						sizeof(*active) * max_active);
	my_group = (int *) my_realloc(my_group, sizeof(int) * max_active);
	unread = (int *) my_realloc(unread, sizeof(int) * max_active);
}


/*
 *  Load the active file into active[]
 */

read_active()
{
	FILE *fp;
	char *p, *q;
	char buf[LEN];
	long h;
	int i;
	extern long hash_groupname();
	FILE *open_active_fp();

	num_active = 0;

	fp = open_active_fp();
	if (fp == NULL) {
		fprintf(stderr, "can't get active file\n");
		exit(1);
	}

	while (fgets(buf, LEN, fp) != NULL) {
		for (p = buf; *p && *p != ' '; p++) ;
		if (*p != ' ') {
			fprintf(stderr, "active file corrupt\n");
			continue;
		}
		*p++ = '\0';

		if (num_active >= max_active)
			expand_active();

		h = hash_groupname(buf);

		if (group_hash[h] == -1)
			group_hash[h] = num_active;
		else {				/* hash linked list chaining */
			for (i = group_hash[h]; active[i].next >= 0;
						i = active[i].next) {
				if (strcmp(active[i].name, buf) == 0)
					goto read_active_continue;
							/* kill dups */
			}
			if (strcmp(active[i].name, buf) == 0)
				goto read_active_continue;
			active[i].next = num_active;
		}

		for (q = p; *q && *q != ' '; q++) ;
		if (*q != ' ') {
			fprintf(stderr, "active file corrupt\n");
			continue;
		}

		active[num_active].name = str_save(buf);
		active[num_active].max = atol(p);
		active[num_active].min = atol(q);
		active[num_active].next = -1;	    /* hash chaining */
		active[num_active].flag = NOTGOT;   /* not in my_group[] yet */

		num_active++;

read_active_continue:;

	}

	fclose(fp);
}



/*
 *  Read $HOME/.newsrc into my_group[].  my_group[] ints point to
 *  active[] entries.  Sub_only determines whether we just read
 *  subscribed groups or all of them.
 */

read_newsrc(sub_only)
int sub_only;		/* TRUE=subscribed groups only, FALSE=all groups */
{
	FILE *fp;
	char *p;
	char buf[8192];
	char c;
	int i;

	local_top = 0;

	fp = fopen(newsrc, "r");
	if (fp == NULL) {		/* attempt to make a .newsrc */
		for (i = 0; i < num_active; i++) {
			if (local_top >= max_active)
				expand_active();
			my_group[local_top] = i;
			active[i].flag = 0;
			unread[local_top] = -1;
			local_top++;
		}
		write_newsrc();
		return;
	}

	while (fgets(buf, 8192, fp) != NULL) {
		p = buf;
		while (*p && *p != '\n' && *p != ' ' && *p != ':' && *p != '!')
			p++;
		c = *p;
		*p++ = '\0';
		if (c == '!' && sub_only)
			continue;		/* unsubscribed */

		if ((i = add_group(buf, FALSE)) < 0) {
		    fprintf(stderr, "group %s not found in active file\n", buf);
		    continue;
		}

		if (c != '!')		/* if we're subscribed to it */
			active[my_group[i]].flag |= SUBS;

		unread[i] = parse_unread(p, my_group[i]);
	}
	fclose(fp);
}


/*
 *  Write a new newsrc from my_group[] and active[]
 *  Used to a create a new .newsrc if there isn't one already, or when
 *  the newsrc is reset.
 */

write_newsrc() {
	FILE *fp;
	int i;

	setuid(real_uid);	/* become the user to write in his */
	setgid(real_gid);	/* home directory */

	fp = fopen(newsrc, "w");
	if (fp == NULL)
		goto write_newsrc_done;

	for (i = 0; i < num_active; i++)
		fprintf(fp, "%s: \n", active[i].name);

	fclose(fp);

write_newsrc_done:
	setuid(tass_uid);
	setgid(tass_gid);
}


/*
 *  Load the sequencer rang lists and mark arts[] according to the
 *  .newsrc info for a particular group.  i.e.	rec.arts.comics: 1-94,97
 */

read_newsrc_line(group)
char *group;
{
	FILE *fp;
	char buf[8192];
	char *p;

	fp = fopen(newsrc, "r");
	if (fp == NULL)
		return;

	while (fgets(buf, 8192, fp) != NULL) {
		p = buf;
		while (*p && *p != '\n' && *p != ' ' && *p != ':' && *p != '!')
			p++;
		*p++ = '\0';
		if (strcmp(buf, group) != 0)
			continue;
		parse_seq(p);
		break;
	}

	fclose(fp);
}


/*
 *  For our current group, update the sequencer information in .newsrc
 */

update_newsrc(group, groupnum)
char *group;
int groupnum;			/* index into active[] for this group */
{
	FILE *fp;
	FILE *newfp;
	char buf[8192];
	char *p;
	char c;
	int gotit = FALSE;

	setuid(real_uid);
	setgid(real_gid);

	fp = fopen(newsrc, "r");
	newfp = fopen(newnewsrc, "w");
	if (newfp == NULL)
		goto update_done;

	if (fp != NULL) {
		while (fgets(buf, 8192, fp) != NULL) {
			for (p = buf; *p; p++)
				if (*p == '\n') {
					*p = '\0';
					break;
				}

			p = buf;
			while (*p && *p != ' ' && *p != ':' && *p != '!')
					p++;
			c = *p;
			if (c != '\0')
				*p++ = '\0';

			if (c != '!')
				c = ':';

			if (strcmp(buf, group) == 0) {
				fprintf(newfp, "%s%c ", buf, c);
				gotit = TRUE;
				print_seq(newfp, groupnum);
				fprintf(newfp, "\n");
			} else
				fprintf(newfp, "%s%c%s\n", buf, c, p);
		}
		fclose(fp);
	}

	fclose(newfp);
	unlink(newsrc);
	link(newnewsrc, newsrc);
	unlink(newnewsrc);

update_done:
	setuid(tass_uid);
	setgid(tass_gid);
}


/*
 *  Subscribe/unsubscribe to a group in .newsrc.  ch should either be
 *  '!' to unsubscribe or ':' to subscribe.  num is the group's index
 *  in active[].
 */

subscribe(group, ch, num, out_seq)
char *group;
char ch;
int num;
int out_seq;				/* output sequencer info? */
{
	FILE *fp;
	FILE *newfp;
	char buf[8192];
	char *p;
	char c;
	int gotit = FALSE;

	if (ch == '!')
		active[num].flag &= ~SUBS;
	else
		active[num].flag |= SUBS;

	setuid(real_uid);
	setgid(real_gid);

	fp = fopen(newsrc, "r");
	newfp = fopen(newnewsrc, "w");
	if (newfp == NULL)
		goto subscribe_done;

	if (fp != NULL) {
		while (fgets(buf, 8192, fp) != NULL) {
			for (p = buf; *p; p++)
				if (*p == '\n') {
					*p = '\0';
					break;
				}

			p = buf;
			while (*p && *p != ' ' && *p != ':' && *p != '!')
					p++;
			c = *p;
			if (c != '\0')
				*p++ = '\0';

			if (c != '!')
				c = ':';

			if (strcmp(buf, group) == 0) {
				fprintf(newfp, "%s%c%s\n", buf, ch, p);
				gotit = TRUE;
			} else
				fprintf(newfp, "%s%c%s\n", buf, c, p);
		}
		fclose(fp);
	}

	if (!gotit) {
		if (out_seq) {
			fprintf(newfp, "%s%c ", group, ch);
			print_seq(newfp, num);
			fprintf(newfp, "\n");
		} else
			fprintf(newfp, "%s%c\n", group, ch);
	}

	fclose(newfp);
	unlink(newsrc);
	link(newnewsrc, newsrc);
	unlink(newnewsrc);

subscribe_done:
	setuid(tass_uid);
	setgid(tass_gid);
}


print_seq(fp, groupnum)
FILE *fp;
int groupnum;			/* index into active[] for this group */
{
	int i;
	int flag = FALSE;

	if (top <= 0) {
		if (active[groupnum].min > 1) {
			fprintf(fp, "1-%ld", active[groupnum].min);
			fflush(fp);
		}
		return;
	}

	i = 0;
	if (arts[0].artnum > 1) {
		for (; i < top && !arts[i].unread; i++) ;
		if (i > 0) {
			if (i == top && arts[i-1].artnum < active[groupnum].max)
				fprintf(fp, "1-%ld", active[groupnum].max);
			else
				fprintf(fp, "1-%ld", arts[i-1].artnum);
		} else
			fprintf(fp, "1-%ld", arts[0].artnum - 1);
		flag = TRUE;
	}

	for (; i < top; i++) {
		if (!arts[i].unread) {
			if (flag)
				fprintf(fp, ",");
			else
				flag = TRUE;
			fprintf(fp, "%ld", arts[i].artnum);
			if (i+1 < top && !arts[i+1].unread) {
				while (i+1 < top && !arts[i+1].unread)
					i++;

				if (i+1 == top
				&& arts[i].artnum < active[groupnum].max)
					fprintf(fp, "-%ld", active[groupnum].max);
				else
					fprintf(fp, "-%ld", arts[i].artnum);
			}
		}
	}

	if (!flag && active[groupnum].min > 1)
		fprintf(fp, "1-%ld", active[groupnum].min);
	fflush(fp);
}


parse_seq(s)
char *s;
{
	long low, high;
	int i;

	while (*s) {
		while (*s && (*s < '0' || *s > '9'))
			s++;

		if (*s && *s >= '0' && *s <= '9') {
			low = atol(s);
			while (*s && *s >= '0' && *s <= '9')
				s++;
			if (*s == '-') {
				s++;
				high = atol(s);
				while (*s && *s >= '0' && *s <= '9')
					s++;
			}  else
				high = low;

			for (i = 0; i < top; i++)
				if (arts[i].artnum >= low &&
				    arts[i].artnum <= high)
					arts[i].unread = 0;
		}
	}
}


parse_unread(s, groupnum)
char *s;
int groupnum;			/* index for group in active[] */
{
	long low, high;
	long last_high;
	int i;
	int sum = 0;
	int gotone = FALSE;
	int n;

/*
 *  Read the first range from the .newsrc sequencer information.  If the
 *  top of the first range is higher than what the active file claims is
 *  the bottom, use it as the new bottom instead
 */

	high = 0;
	if (*s) {
		while (*s && (*s < '0' || *s > '9'))
			s++;

		if (*s && *s >= '0' && *s <= '9') {
			low = atol(s);
			while (*s && *s >= '0' && *s <= '9')
				s++;
			if (*s == '-') {
				s++;
				high = atol(s);
				while (*s && *s >= '0' && *s <= '9')
					s++;
			}  else
				high = low;
			gotone = TRUE;
		}
	}

	if (high < active[groupnum].min)
		high = active[groupnum].min;

	while (*s) {
		last_high = high;

		while (*s && (*s < '0' || *s > '9'))
			s++;

		if (*s && *s >= '0' && *s <= '9') {
			low = atol(s);
			while (*s && *s >= '0' && *s <= '9')
				s++;
			if (*s == '-') {
				s++;
				high = atol(s);
				while (*s && *s >= '0' && *s <= '9')
					s++;
			}  else
				high = low;

			if (low > last_high)	/* otherwise seq out of order */
				sum += (low - last_high) - 1;
		}
	}

	if (gotone) {
		if (active[groupnum].max > high)
			sum += active[groupnum].max - high;
		return sum;
	}

	n = (int) (active[groupnum].max - active[groupnum].min);
	if (n < 2)
		return 0;

	return -1;
}


get_line_unread(group, groupnum)
char *group;
int groupnum;				/* index for group in active[] */
{
	FILE *fp;
	char buf[8192];
	char *p;
	int ret = -1;

	fp = fopen(newsrc, "r");
	if (fp == NULL)
		return -1;

	while (fgets(buf, 8192, fp) != NULL) {
		p = buf;
		while (*p && *p != '\n' && *p != ' ' && *p != ':' && *p != '!')
			p++;
		*p++ = '\0';
		if (strcmp(buf, group) != 0)
			continue;
		ret = parse_unread(p, groupnum);
		break;
	}

	fclose(fp);
	return ret;
}


reset_newsrc()
{
	FILE *fp;
	FILE *newfp;
	char buf[8192];
	char *p;
	char c;
	int gotit = FALSE;
	int i;

	setuid(real_uid);
	setgid(real_gid);

	fp = fopen(newsrc, "r");
	newfp = fopen(newnewsrc, "w");
	if (newfp == NULL)
		goto update_done;

	if (fp != NULL) {
		while (fgets(buf, 8192, fp) != NULL) {
			for (p = buf; *p && *p != '\n'; p++) ;
			*p = '\0';

			p = buf;
			while (*p && *p != ' ' && *p != ':' && *p != '!')
					p++;
			c = *p;
			if (c != '\0')
				*p++ = '\0';

			if (c != '!')
				c = ':';

			fprintf(newfp, "%s%c\n", buf, c);
		}
		fclose(fp);
	}

	fclose(newfp);
	unlink(newsrc);
	link(newnewsrc, newsrc);
	unlink(newnewsrc);

update_done:
	setuid(tass_uid);
	setgid(tass_gid);

	for (i = 0; i < local_top; i++)
		unread[i] = -1;
}


delete_group(group)
char *group;
{
	FILE *fp;
	FILE *newfp;
	char buf[8192];
	char *p;
	char c;
	int gotit = FALSE;
	FILE *del;

	setuid(real_uid);
	setgid(real_gid);

	fp = fopen(newsrc, "r");
	newfp = fopen(newnewsrc, "w");
	if (newfp == NULL)
		goto del_done;
	del = fopen(delgroups, "a+");
	if (del == NULL)
		goto del_done;

	if (fp != NULL) {
		while (fgets(buf, 8192, fp) != NULL) {
			for (p = buf; *p && *p != '\n'; p++) ;
			*p = '\0';

			p = buf;
			while (*p && *p != ' ' && *p != ':' && *p != '!')
					p++;
			c = *p;
			if (c != '\0')
				*p++ = '\0';

			if (c != '!')
				c = ':';

			if (strcmp(buf, group) == 0) {
				fprintf(del, "%s%c%s\n", buf, c, p);
				gotit = TRUE;
			} else
				fprintf(newfp, "%s%c%s\n", buf, c, p);
		}
		fclose(fp);
	}

	fclose(newfp);

	if (!gotit)
		fprintf(del, "%s! \n", group);

	fclose(del);
	unlink(newsrc);
	link(newnewsrc, newsrc);
	unlink(newnewsrc);

del_done:
	setuid(tass_uid);
	setgid(tass_gid);
}


undel_group() {
	FILE *del;
	FILE *newfp;
	FILE *fp;
	char buf[2][8192];
	char *p;
	int which = 0;
	long h;
	extern int cur_groupnum;
	int i, j;
	char c;

	setuid(real_uid);
	setgid(real_gid);

	del = fopen(delgroups, "r");
	if (del == NULL) {
		setuid(tass_uid);
		setgid(tass_gid);
		return FALSE;
	}
	unlink(delgroups);
	newfp = fopen(delgroups, "w");
	if (newfp == NULL) {
		setuid(tass_uid);
		setgid(tass_gid);
		return FALSE;
	}

	buf[0][0] = '\0';
	buf[1][0] = '\0';

	while (fgets(buf[which], 8192, del) != NULL) {
		which = !which;
		if (*buf[which])
			fputs(buf[which], newfp);
	}

	fclose(del);
	fclose(newfp);
	which = !which;

	if (!*buf[which]) {
		setuid(tass_uid);
		setgid(tass_gid);
		return FALSE;
	}

	for (p = buf[which]; *p && *p != '\n'; p++) ;
	*p = '\0';

	p = buf[which];
	while (*p && *p != ' ' && *p != ':' && *p != '!')
		p++;
	c = *p;
	if (c != '\0')
		*p++ = '\0';

	if (c != '!')
		c = ':';

	{			/* find the hash of the group name */
		char *t = buf[which];

		h = *t++;
		while (*t)
			h = (h * 64 + *t++) % TABLE_SIZE;
	}

	for (i = group_hash[h]; i >= 0; i = active[i].next) {
		if (strcmp(buf[which], active[i].name) == 0) {
			for (j = 0; j < local_top; j++)
				if (my_group[j] == i) {
					setuid(tass_uid);
					setgid(tass_gid);
					return j;
				}

			active[i].flag &= ~NOTGOT;   /* mark that we got it */
			if (c != '!')
				active[i].flag |= SUBS;

			if (local_top >= max_active)
				expand_active();
			local_top++;
			for (j = local_top; j > cur_groupnum; j--) {
				my_group[j] = my_group[j-1];
				unread[j] = unread[j-1];
			}
			my_group[cur_groupnum] = i;
			unread[cur_groupnum] = parse_unread(p, i);

			fp = fopen(newsrc, "r");
			if (fp == NULL) {
				setuid(tass_uid);
				setgid(tass_gid);
				return FALSE;
			}
			newfp = fopen(newnewsrc, "w");
			if (newfp == NULL) {
				fclose(fp);
				setuid(tass_uid);
				setgid(tass_gid);
				return FALSE;
			}
			i = 0;
			while (fgets(buf[!which], 8192, fp) != NULL) {
				for (p = buf[!which]; *p && *p != '\n'; p++) ;
				*p = '\0';

				p = buf[!which];
				while (*p && *p!=' ' && *p != ':' && *p != '!')
					p++;
				c = *p;
				if (c != '\0')
					*p++ = '\0';

				if (c != '!')
					c = ':';

				while (i < cur_groupnum) {
					if (strcmp(buf[!which],
					  active[my_group[i]].name) == 0) {
						fprintf(newfp, "%s%c%s\n",
							buf[!which], c, p);
						goto foo_cont;
					}
					i++;
				}
				fprintf(newfp, "%s%c%s\n", buf[which], c, p);
				fprintf(newfp, "%s%c%s\n", buf[!which], c, p);
				break;
foo_cont:;
			}

			while (fgets(buf[!which], 8192, fp) != NULL)
				fputs(buf[!which], newfp);

			fclose(newfp);
			fclose(fp);
			unlink(newsrc);
			link(newnewsrc, newsrc);
			unlink(newnewsrc);
			setuid(tass_uid);
			setgid(tass_gid);
			return TRUE;
		}
	}

	setuid(tass_uid);
	setgid(tass_gid);

	return FALSE;
}


mark_group_read(group, groupnum)
char *group;
int groupnum;			/* index into active[] for this group */
{
	FILE *fp;
	FILE *newfp;
	char buf[8192];
	char *p;
	char c;
	int gotit = FALSE;

	if (active[groupnum].max < 2)
		return;

	setuid(real_uid);
	setgid(real_gid);

	fp = fopen(newsrc, "r");
	newfp = fopen(newnewsrc, "w");
	if (newfp == NULL)
		goto mark_group_read_done;

	if (fp != NULL) {
		while (fgets(buf, 8192, fp) != NULL) {
			for (p = buf; *p; p++)
				if (*p == '\n') {
					*p = '\0';
					break;
				}

			p = buf;
			while (*p && *p != ' ' && *p != ':' && *p != '!')
					p++;
			c = *p;
			if (c != '\0')
				*p++ = '\0';

			if (c != '!')
				c = ':';

			if (strcmp(buf, group) == 0) {
				fprintf(newfp, "%s%c 1-%ld\n", buf, c,
						active[groupnum].max);
				gotit = TRUE;
			} else
				fprintf(newfp, "%s%c%s\n", buf, c, p);
		}
		fclose(fp);
	}

	fclose(newfp);
	unlink(newsrc);
	link(newnewsrc, newsrc);
	unlink(newnewsrc);

mark_group_read_done:
	setuid(tass_uid);
	setgid(tass_gid);
}


long
hash_groupname(buf)		/* hash group name for fast lookup later */
char *buf;
{
	char *t = buf;
	unsigned long h;

	h = *t++;
	while (*t)
		h = ((h << 1) ^ *t++) % TABLE_SIZE;
/*		h = (h * 64 + *t++) % TABLE_SIZE;	*/

	return h;
}


#ifdef M_XENIX
mkdir(path, mode)
char *path;
int mode;
{
	char buf[LEN];

	sprintf(buf, "mkdir %s 2> /dev/null", path);
	system(buf);
	chmod(path, mode);
}
#endif


#ifdef XTRA
mkdir_p(path)
char *path;
{
	char *p;
	char c;

	p = path;

	do {
		while (*p && *p != '/')
			p++;

		c = *p;
		*p = '\0';

		mkdir(path, 0755);

		*p++ = c;
	} while (c);
}
#endif


switch_chars(ch)
char ch;
{

	switch (ch) {
	case ' ':	return '\t';
	case '\t':	return ' ';
	case 'n':	return 'N';
	case 'N':	return 'n';
	case 'p':	return 'P';
	case 'P':	return 'p';
	}

	return ch;
}

